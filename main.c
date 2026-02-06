#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Entry {
  void *data;
  struct Entry *next;
} Entry;

typedef struct Bucket {
  int local_depth;
  int size;
  struct Entry *head;
} Bucket;

typedef struct HashMap {
  int global_depth;
  int bucket_limit;
  int dir_size;
  Bucket **directory;

  int (*getIndex)(const void *data, int depth);
  int (*comparator)(const void *a, const void *b);
} HashMap;

Bucket *bucket_create(int local_depth) {
  Bucket *b = malloc(sizeof(Bucket));
  if (!b)
    return NULL;

  b->local_depth = local_depth;
  b->size = 0;
  b->head = NULL;
  return b;
}

void bucket_insert(Bucket *bucket, void *data) {
  if (!bucket || !data)
    return;

  Entry *e = malloc(sizeof(Entry));
  if (!e)
    return;

  e->data = data;
  e->next = bucket->head;
  bucket->head = e;
  bucket->size++;
}

void bucket_clear(Bucket *bucket) {
  if (!bucket)
    return;

  Entry *curr = bucket->head;
  while (curr) {
    Entry *next = curr->next;

    free(curr->data);
    free(curr);

    curr = next;
  }

  bucket->head = NULL;
  bucket->size = 0;
}

static void doubleDirectory(HashMap *map) {
  int old_size = map->dir_size;

  map->dir_size *= 2;
  map->global_depth++;

  map->directory = realloc(map->directory, sizeof(Bucket *) * map->dir_size);

  for (int i = 0; i < old_size; i++) {
    map->directory[i + old_size] = map->directory[i];
  }
}

static void splitBucket(HashMap *map, int dir_index) {
  Bucket *old = map->directory[dir_index];

  if (old->local_depth == map->global_depth) {
    doubleDirectory(map);
  }

  Bucket *new_bucket = bucket_create(old->local_depth + 1);
  old->local_depth++;

  int bit = 1 << (old->local_depth - 1);

  for (int i = 0; i < map->dir_size; i++) {
    if (map->directory[i] == old && (i & bit)) {
      map->directory[i] = new_bucket;
    }
  }

  Entry *curr = old->head;
  old->head = NULL;
  old->size = 0;

  while (curr) {
    int idx = map->getIndex(curr->data, map->global_depth);

    bucket_insert(map->directory[idx], curr->data);
    free(curr);

    curr = curr->next;
  }
}

HashMap *hashmap_create(int bucket_limit, void *getIndexFunction,
                        void *comparator) {
  HashMap *map = malloc(sizeof(HashMap));
  if (!map)
    return NULL;

  map->global_depth = 1;
  map->bucket_limit = bucket_limit;
  map->dir_size = 2;
  map->getIndex = getIndexFunction;
  map->comparator = comparator;

  map->directory = malloc(sizeof(Bucket *) * map->dir_size);

  Bucket *b0 = bucket_create(1);
  Bucket *b1 = bucket_create(1);

  map->directory[0] = b0;
  map->directory[1] = b1;

  return map;
}

void *hashmap_find(HashMap *map, void *data) {
  if (!map || !data)
    return NULL;

  int idx = map->getIndex(data, map->global_depth);
  Bucket *b = map->directory[idx];

  for (Entry *e = b->head; e; e = e->next) {
    if (map->comparator(e->data, data))
      return e->data;
  }

  return NULL;
}

void hashmap_insert(HashMap *map, void *data) {
  if (!map || !data)
    return;

  if (hashmap_find(map, data))
    return;

  int idx = map->getIndex(data, map->global_depth);
  Bucket *b = map->directory[idx];

  if (b->size < map->bucket_limit) {
    bucket_insert(b, data);
    return;
  }

  splitBucket(map, idx);
  hashmap_insert(map, data);
}

void hashmap_destroy(HashMap *map) {
  if (!map)
    return;

  int size = map->dir_size;

  for (int i = 0; i < size; i++) {
    int unique = 1;
    for (int j = 0; j < i; j++) {
      if (map->directory[i] == map->directory[j]) {
        unique = 0;
        break;
      }
    }

    if (unique) {
      bucket_clear(map->directory[i]);
      free(map->directory[i]);
    }
  }

  free(map->directory);
  free(map);
}

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

Token *token_create(char token_name[], int row, int col, int index,
                    char type[]) {
  Token *tk = malloc(sizeof(Token));
  tk->row = row;
  tk->col = col;
  tk->index = index;
  strcpy(tk->token_name, token_name);
  strcpy(tk->type, type);
  return tk;
}

Symbol *symbol_create(char *lexeme, int size, char *type, char *scope) {
  Symbol *sym = malloc(sizeof(Symbol));

  strcpy(sym->lexeme, lexeme);
  strcpy(sym->type, type);
  strcpy(sym->scope, scope);
  sym->size = size;

  return sym;
}

int symbol_compare(const Symbol *a, const Symbol *b) {
  return strcmp(a->lexeme, b->lexeme) == 0;
}

int symbol_getIndex(const Symbol *sym, int depth) {
  unsigned hash = 5381;
  const char *lexeme = sym->lexeme;
  while (*lexeme)
    hash = ((hash << 5) + hash) + *lexeme++;
  return hash & ((1 << depth) - 1);
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
    return token_create(tok, start_row, start_col, -1, "PUNCT");
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

  return token_create(buf, start_row, start_col, index, "IDENTIFIER");
}

Token *isKeyword(FILE *fp, int *row, int *col) {
  static const char *keywords[] = {
      "auto",     "break",    "case",     "char",   "const",   "continue",
      "default",  "do",       "double",   "else",   "enum",    "extern",
      "float",    "for",      "goto",     "if",     "inline",  "int",
      "long",     "register", "restrict", "return", "short",   "signed",
      "sizeof",   "static",   "struct",   "switch", "typedef", "union",
      "unsigned", "void",     "volatile", "while",  "FILE",    "size_t"};

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

Token *isStringLiteral(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char buf[1024];
  int len = 0;

  int c = nextChar(fp, row, col);

  if (c != '"') {
    ungetChar(c, fp, row, col);
    return NULL;
  }

  buf[len++] = '"';

  while ((c = nextChar(fp, row, col)) != EOF) {

    if (len >= sizeof(buf) - 1)
      break;

    buf[len++] = c;

    if (c == '\\') {
      int next = nextChar(fp, row, col);
      if (next == EOF)
        break;

      if (len >= sizeof(buf) - 1)
        break;

      buf[len++] = next;
      continue;
    }

    if (c == '"') {
      buf[len] = '\0';
      return token_create(buf, start_row, start_col, -1, "STRING");
    }
  }

  buf[len] = '\0';
  return token_create(buf, start_row, start_col, -1, "BAD_STRING");
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

  return token_create(buf, start_row, start_col, -1, "NUM");
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
    return token_create(buf, start_row, start_col, -1, "RELOP");
  }

  ungetChar(n, fp, row, col);

  if (c == '<' || c == '>') {
    buf[0] = c;
    return token_create(buf, start_row, start_col, -1, "RELOP");
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
    return token_create(buf, start_row, start_col, -1, "ASSIGN");
  }

  ungetChar(n, fp, row, col);

  if (c == '=') {
    buf[0] = '=';
    return token_create(buf, start_row, start_col, -1, "ASSIGN");
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
    return token_create(buf, start_row, start_col, -1, "ADDOP");
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
    return token_create(buf, start_row, start_col, -1, "MULOP");
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
    return token_create("EOF", row, col, -1, "EOF");

  ungetChar(c, fp, &row, &col);

  Token *tok;
  if ((tok = isKeyword(fp, &row, &col)))
    return tok;
  if ((tok = isIdentifier(fp, &row, &col, index)))
    return index++, tok;
  if ((tok = isStringLiteral(fp, &row, &col)))
    return tok;
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
  return token_create(unk, row, col - 1, -1, "UNKNOWN");
}

void skipCommentsAndDirectives(FILE *fp) {
  FILE *out = fopen("temp.c", "w");

  int c, next;
  int in_string = 0, in_char = 0;
  int start_of_line = 1;

  while ((c = fgetc(fp)) != EOF) {

    if (!in_char && c == '"' && !in_string) {
      in_string = 1;
      fputc(c, out);
      continue;
    } else if (in_string) {
      fputc(c, out);

      if (c == '\\') {
        int esc = fgetc(fp);
        if (esc != EOF)
          fputc(esc, out);
        continue;
      }

      if (c == '"')
        in_string = 0;

      continue;
    }

    if (!in_string && c == '\'' && !in_char) {
      in_char = 1;
      fputc(c, out);
      continue;
    } else if (in_char) {
      fputc(c, out);

      if (c == '\\') {
        int esc = fgetc(fp);
        if (esc != EOF)
          fputc(esc, out);
        continue;
      }

      if (c == '\'')
        in_char = 0;

      continue;
    }

    if (!in_string && !in_char && start_of_line) {
      int temp = c;

      while (isspace(temp) && temp != '\n') {
        fputc(temp, out);
        temp = fgetc(fp);
      }

      if (temp == '#') {
        while ((c = fgetc(fp)) != EOF && c != '\n')
          ;
        fputc('\n', out);
        start_of_line = 1;
        continue;
      }

      ungetc(temp, fp);
      c = fgetc(fp);
      start_of_line = 0;
    }

    if (!in_string && !in_char && c == '/') {
      next = fgetc(fp);

      if (next == '/') {
        while ((c = fgetc(fp)) != EOF && c != '\n')
          ;
        fputc('\n', out);
        start_of_line = 1;
        continue;
      }

      if (next == '*') {
        int prev = 0;
        while ((c = fgetc(fp)) != EOF) {
          if (prev == '*' && c == '/')
            break;
          prev = c;
        }
        continue;
      }

      ungetc(next, fp);
    }

    fputc(c, out);

    start_of_line = (c == '\n');
  }

  fclose(out);
}

static void displayHashMap(HashMap *map) {
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
      printf("hash: %-4d | lexeme: %-15s | size: %-4d | type: %-12s | scope: "
             "%-8s\n",
             i, s->lexeme, s->size, s->type, s->scope);
    }
  }

  printf("\n====================\n");
}

static int isTypeKeyword(const char *s) {
  static const char *types[] = {"int",  "void",  "char",   "double",  "float",
                                "long", "short", "signed", "unsigned"};
  static const int sizes[] = {sizeof(int),     0,
                              sizeof(char),    sizeof(double),
                              sizeof(float),   sizeof(long),
                              sizeof(short),   sizeof(int),
                              sizeof(unsigned)};

  for (int i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
    if (strcmp(s, types[i]) == 0)
      return sizes[i];
  }

  return -1;
}

static int isBuiltin(const char *s) {
  static const char *funcs[] = {"printf", "scanf",  "fopen",  "fclose",
                                "malloc", "calloc", "strlen", "strcpy"};

  for (int i = 0; i < sizeof(funcs) / sizeof(funcs[0]); i++) {
    if (strcmp(s, funcs[i]) == 0)
      return 1;
  }

  return 0;
}

void compile(const char *input_file) {

  FILE *fp = fopen(input_file, "r");
  skipCommentsAndDirectives(fp);
  fclose(fp);

  FILE *temp_fp = fopen("temp.c", "r");

  HashMap *map = hashmap_create(3, symbol_getIndex, symbol_compare);

  Token *prev = NULL;
  Token *curr = getNextToken(temp_fp);

  char last_type[64] = "";
  int last_type_row = -1;

  while (curr && strcmp(curr->type, "EOF") != 0) {
    printf("<%s, %d, %d, %d, %s>\n", curr->token_name, curr->row, curr->col,
           curr->index, curr->type);

    if (strcmp(curr->type, "KEYWORD") == 0 &&
        isTypeKeyword(curr->token_name) != -1) {
      strcpy(last_type, curr->token_name);
      last_type_row = curr->row;
    } else if (strcmp(curr->type, "IDENTIFIER") == 0) {
      Token *peek = getNextToken(temp_fp);
      int is_function = 0;

      if (peek && strcmp(peek->type, "PUNCT") == 0 &&
          peek->token_name[0] == '(')
        is_function = 1;

      if (curr->row == last_type_row) {
        if (is_function) {
          hashmap_insert(map, symbol_create(curr->token_name,
                                            isTypeKeyword(last_type),
                                            "function", "global"));

          prev = peek;
          curr = getNextToken(temp_fp);
          continue;
        }

        hashmap_insert(map,
                       symbol_create(curr->token_name, isTypeKeyword(last_type),
                                     last_type, "global"));
      } else if (is_function) {
        hashmap_insert(
            map, symbol_create(curr->token_name, -1, "function", "global"));

        prev = peek;
        curr = getNextToken(temp_fp);
        continue;
      } else if (isBuiltin(curr->token_name)) {
        hashmap_insert(
            map, symbol_create(curr->token_name, -1, "function", "global"));
      }
    }

    if (strcmp(curr->type, "PUNCT") == 0 && curr->token_name[0] == ';') {
      last_type[0] = 0;
      last_type_row = -1;
    }

    prev = curr;
    curr = getNextToken(temp_fp);
  }

  displayHashMap(map);

  fclose(temp_fp);
  hashmap_destroy(map);
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
