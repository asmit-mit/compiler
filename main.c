#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.c"

typedef struct Token {
  char token_name[100];
  int index;
  unsigned int row, col;
  char type[100];
} Token;

typedef struct Symbol {
    char lexeme[20];
    int size;
    char type[20];
    char scope[20];
} Symbol;

Token *getToken(char token_name[], int row, int col, int index, char type[]) {
  Token* tk = malloc(sizeof(Token));
  tk->row = row;
  tk->col = col;
  tk->index = index;
  strcpy(tk->token_name, token_name);
  strcpy(tk->type, type);
  return tk;
}

Symbol *getSymbol(char *lexeme, int size, char *type, char *scope) {
  Symbol *sym = malloc(sizeof(Symbol));

  strcpy(sym->lexeme, lexeme);
  strcpy(sym->type, type);
  strcpy(sym->scope, scope);
  sym->size = size;

  return sym;
}

int compareSymbol(const Symbol *a, const Symbol *b) {
  return strcmp(a->lexeme, b->lexeme) == 0;
}

int getIndex(const Symbol *sym, int depth) {
  unsigned hash = 5381;
  const char *lexeme = sym->lexeme;
  while (*lexeme)
    hash = ((hash << 5) + hash) + *lexeme++;
  return hash & ((1 << depth) - 1);
}

void skipCommentsDirectivesAndStrings(FILE *fp) {
  FILE *out = fopen("temp.c", "w");
  if (!out) {
    perror("temp.c");
    return;
  }

  int c, next;
  int in_string = 0, in_char = 0;
  int start_of_line = 1;

  while ((c = fgetc(fp)) != EOF) {

    if (start_of_line) {
      int temp = c;

      while (isspace(temp) && temp != '\n') {
        fputc(temp, out);
        temp = fgetc(fp);
      }

      if (temp == '#') {
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '\n') {
            fputc('\n', out);
            break;
          }
          fputc(' ', out);
        }
        start_of_line = 1;
        continue;
      }

      ungetc(temp, fp);
      c = fgetc(fp);
      start_of_line = 0;
    }

    if (c == '"' && !in_char) {
      in_string = 1;
      fputc('"', out);

      while ((c = fgetc(fp)) != EOF) {
        if (c == '\\') {
          fputc(' ', out);
          fputc(' ', out);
          fgetc(fp);
          continue;
        }
        if (c == '"') {
          fputc('"', out);
          in_string = 0;
          break;
        }
        if (c == '\n')
          fputc('\n', out);
        else
          fputc(' ', out);
      }
      continue;
    }

    if (c == '\'' && !in_string) {
      in_char = 1;
      fputc('\'', out);

      while ((c = fgetc(fp)) != EOF) {
        if (c == '\\') {
          fputc(' ', out);
          fputc(' ', out);
          fgetc(fp);
          continue;
        }
        if (c == '\'') {
          fputc('\'', out);
          in_char = 0;
          break;
        }
        if (c == '\n')
          fputc('\n', out);
        else
          fputc(' ', out);
      }
      continue;
    }

    if (!in_string && !in_char && c == '/') {
      next = fgetc(fp);

      if (next == '/') {
        fputc(' ', out);
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '\n') {
            fputc('\n', out);
            break;
          }
          fputc(' ', out);
        }
        start_of_line = 1;
        continue;
      }

      if (next == '*') {
        fputc(' ', out);
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '*' && (next = fgetc(fp)) == '/') {
            fputc(' ', out);
            fputc(' ', out);
            break;
          }
          if (c == '\n')
            fputc('\n', out);
          else
            fputc(' ', out);
        }
        continue;
      }

      ungetc(next, fp);
    }

    fputc(c, out);

    if (c == '\n')
      start_of_line = 1;
  }

  fclose(out);
}

static int prev_row;

char nextChar(FILE *fp, int *row, int *col) {
  int c = fgetc(fp);
  if (c == EOF)
    return EOF;

  prev_row = *row;

  if (c == '\n') {
    (*row)++;
    *col = 1;
  } else {
    (*col)++;
  }

  return c;
}

char ungetChar(char c, FILE *fp, int *row, int *col) {
  if (c == EOF)
    return EOF;

  ungetc(c, fp);

  *row = prev_row;

  if (c == '\n') {
    *col = 1;
  } else if (c == '\t') {
    *col -= 2;
    if (*col < 1)
      *col = 1;
  } else {
    (*col)--;
    if (*col < 1)
      *col = 1;
  }

  return c;
}

Token *isPunctuation(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char tok[2] = {c, 0};

  switch (c) {
  case '(':
  case ')':
  case '{':
  case '}':
  case '[':
  case ']':
  case ';':
  case ',':
  case '.':
  case '"':
  case '\'':
  case '\\':
    return getToken(tok, start_row, start_col, -1, "PUNCT");
  default:
    ungetChar(c, fp, row, col);
    return NULL;
  }
}

Token *isIdentifier(FILE *fp, int *row, int *col, int index) {
  int start_row = *row;
  int start_col = *col;

  char buf[128];
  int len = 0;

  char c = nextChar(fp, row, col);
  if (!(isalpha(c) || c == '_')) {
    ungetChar(c, fp, row, col);
    return NULL;
  }

  buf[len++] = c;

  while ((c = nextChar(fp, row, col)) != EOF && (isalnum(c) || c == '_')) {
    buf[len++] = c;
  }

  buf[len] = 0;
  ungetChar(c, fp, row, col);

  return getToken(buf, start_row, start_col, index, "IDENTIFIER");
}

Token *isKeyword(FILE *fp, int *row, int *col) {
  static const char *keywords[] = {
      "auto",     "break",    "case",     "char",   "const",   "continue",
      "default",  "do",       "double",   "else",   "enum",    "extern",
      "float",    "for",      "goto",     "if",     "inline",  "int",
      "long",     "register", "restrict", "return", "short",   "signed",
      "sizeof",   "static",   "struct",   "switch", "typedef", "union",
      "unsigned", "void",     "volatile", "while", "FILE", "size_t"};

  long pos = ftell(fp);
  int r = *row, c = *col;

  Token *tok = isIdentifier(fp, row, col, -1);
  if (!tok)
    return NULL;

  for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    if (strcmp(tok->token_name, keywords[i]) == 0) {
      strcpy(tok->type, "KEYWORD");
      return tok;
    }
  }

  fseek(fp, pos, SEEK_SET);
  *row = r;
  *col = c;
  return NULL;
}

Token *isNum(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char buf[128];
  int len = 0, dot = 0, exp = 0;

  char c = nextChar(fp, row, col);
  if (!isdigit(c)) {
    ungetChar(c, fp, row, col);
    return NULL;
  }

  buf[len++] = c;

  while ((c = nextChar(fp, row, col)) != EOF) {
    if (isdigit(c)) {
      buf[len++] = c;
    } else if (c == '.' && !dot) {
      dot = 1;
      buf[len++] = c;
    } else if ((c == 'e' || c == 'E') && !exp) {
      exp = 1;
      buf[len++] = c;
      c = nextChar(fp, row, col);
      if (c == '+' || c == '-' || isdigit(c)) {
        buf[len++] = c;
      } else {
        ungetChar(c, fp, row, col);
        break;
      }
    } else
      break;
  }

  buf[len] = 0;
  ungetChar(c, fp, row, col);

  return getToken(buf, start_row, start_col, -1, "NUM");
}

Token *isRelop(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char n = nextChar(fp, row, col);
  char buf[3] = {0};

  if ((c == '<' || c == '>' || c == '=' || c == '!') && n == '=') {
    buf[0] = c;
    buf[1] = '=';
    return getToken(buf, start_row, start_col, -1, "RELOP");
  }

  ungetChar(n, fp, row, col);

  if (c == '<' || c == '>') {
    buf[0] = c;
    return getToken(buf, start_row, start_col, -1, "RELOP");
  }

  ungetChar(c, fp, row, col);
  return NULL;
}

Token *isAssignop(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char n = nextChar(fp, row, col);
  char buf[3] = {0};

  if (n == '=' && strchr("+-*/%", c)) {
    buf[0] = c;
    buf[1] = '=';
    return getToken(buf, start_row, start_col, -1, "ASSIGN");
  }

  ungetChar(n, fp, row, col);

  if (c == '=') {
    buf[0] = '=';
    return getToken(buf, start_row, start_col, -1, "ASSIGN");
  }

  ungetChar(c, fp, row, col);
  return NULL;
}

Token *isAddop(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char buf[2] = {c, '\0'};

  if (c == '+' || c == '-') {
    return getToken(buf, start_row, start_col, -1, "ADDOP");
  }

  ungetChar(c, fp, row, col);
  return NULL;
}

Token *isMulop(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char buf[2] = {c, '\0'};

  if (c == '*' || c == '/' || c == '%') {
    return getToken(buf, start_row, start_col, -1, "MULOP");
  }

  ungetChar(c, fp, row, col);
  return NULL;
}

Token *getNextToken(FILE *fp) {
  static int row = 1, col = 1;
  static int index = 0;

  char c;

  while ((c = nextChar(fp, &row, &col)) != EOF) {
    if (isspace(c))
      continue;
    break;
  }

  if (c == EOF)
    return getToken("EOF", row, col, -1, "EOF");

  ungetChar(c, fp, &row, &col);

  Token *tok;
  if ((tok = isKeyword(fp, &row, &col)))
    return tok;
  if ((tok = isIdentifier(fp, &row, &col, index)))
    return index++, tok;
  if ((tok = isNum(fp, &row, &col)))
    return tok;
  if ((tok = isRelop(fp, &row, &col)))
    return tok;
  if ((tok = isAssignop(fp, &row, &col)))
    return tok;
  if ((tok = isAddop(fp, &row, &col)))
    return tok;
  if ((tok = isMulop(fp, &row, &col)))
    return tok;
  if ((tok = isPunctuation(fp, &row, &col)))
    return tok;

  c = nextChar(fp, &row, &col);
  char unk[2] = {c, '\0'};
  return getToken(unk, row, col - 1, -1, "UNKNOWN");
}

typedef struct {
  char *name;
  char *ret_type;
  int size;
} BuiltinFunc;

static BuiltinFunc builtin_funcs[] = {
    {"printf", "int", 4},   {"scanf", "int", 4},     {"malloc", "void*", 8},
    {"free", "void", 0},    {"strlen", "size_t", 8}, {"strcmp", "int", 4},
    {"strcpy", "char*", 8}, {"exit", "void", 0},     {"fopen", "FILE*", 8},
    {"fclose", "int", 4},   {"main", "int", 4},      {"compile", "void", 0}};

static const int builtin_count =
    sizeof(builtin_funcs) / sizeof(builtin_funcs[0]);

static BuiltinFunc *findBuiltin(const char *name) {
  for (int i = 0; i < builtin_count; i++) {
    if (strcmp(builtin_funcs[i].name, name) == 0)
      return &builtin_funcs[i];
  }
  return NULL;
}

typedef struct {
  char *type;
  int size;
} PrimitiveType;

static PrimitiveType primitive_types[] = {
    {"char", 1},  {"int", 4},  {"float", 4},  {"double", 8},  {"void", 0},
    {"char*", 8}, {"int*", 8}, {"float*", 8}, {"double*", 8}, {"void*", 8}};

static const int primitive_count =
    sizeof(primitive_types) / sizeof(primitive_types[0]);

static int getPrimitiveSize(const char *type) {
  for (int i = 0; i < primitive_count; i++) {
    if (strcmp(primitive_types[i].type, type) == 0)
      return primitive_types[i].size;
  }
  return -1; /* not a primitive */
}

void displayHashMap(HashMap *map) {
  if (!map)
    return;

  printf("\n=== Symbol Table ===\n\n");

  for (int i = 0; i < map->dir_size; i++) {
    Bucket *b = map->directory[i];

    int seen = 0;
    for (int j = 0; j < i; j++) {
      if (map->directory[j] == b) {
        seen = 1;
        break;
      }
    }

    if (seen)
      continue;

    for (Entry *e = b->head; e; e = e->next) {
      Symbol *s = (Symbol *)e->data;
      printf("lexeme: %-12s | size: %d | type: %s, | rtype: %s\n", s->lexeme,
             s->size, s->type, s->scope);
    }
  }

  printf("\n====================\n");
}

void compile(const char *input_file) {
  FILE *fp = fopen(input_file, "r");
  skipCommentsDirectivesAndStrings(fp);
  fclose(fp);

  FILE *temp_fp = fopen("temp.c", "r");

  HashMap *map = createHashMap(3, getIndex, compareSymbol);

  Token *tok, *prev_tok;
  do {
    if (tok && strcmp(tok->type, "KEYWORD") == 0)
      prev_tok = tok;
    tok = getNextToken(temp_fp);
    if (tok == NULL) {
      fprintf(stderr, "Lexer error\n");
      break;
    }

    printf("<%s, %d, %d, %d, %s>\n", tok->token_name, tok->row, tok->col,
           tok->index, tok->type);

    if (strcmp(tok->type, "IDENTIFIER") == 0) {

      BuiltinFunc *bf = findBuiltin(tok->token_name);

      if (bf) {
        insertHashMap(map, getSymbol(bf->name, bf->size, "func", bf->ret_type));
      } else {
        char *decl_type = "void";
        int size = 0;

        if (prev_tok) {
          int prim_size = getPrimitiveSize(prev_tok->token_name);
          if (prim_size >= 0) {
            decl_type = prev_tok->token_name;
            size = prim_size;
          }
        }

        insertHashMap(map, getSymbol(tok->token_name, size, decl_type, ""));
      }
    }

  } while (strcmp(tok->type, "EOF") != 0);

  displayHashMap(map);
  destroyHashMap(map);

  fclose(temp_fp);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Use as ./compile <input-file-location>\n");
    return 0;
  }

  char *input_file = argv[1];
  compile(input_file);

  return 0;
}
