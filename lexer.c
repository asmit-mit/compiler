#pragma once

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "stack.c"

typedef struct Token {
  char token_name[100];
  int index;
  unsigned int row, col;
  char type[100];
} Token;

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

typedef struct Symbol {
  char lexeme[20];
  int size;
  char type[20];
  char scope[20];
} Symbol;

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

typedef struct Position {
  int row, col;
} Position;

static Position *getPosition(int row, int col) {
  Position *pos = (Position *)malloc(sizeof(Position));
  pos->row = row;
  pos->col = col;
  return pos;
}

Stack *stack;

char nextChar(FILE *fp, int *row, int *col) {
  int c = fgetc(fp);
  if (c == EOF)
    return EOF;

  stack_push(stack, getPosition(*row, *col));

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

  Position *p = (Position *)stack_top(stack);
  if (p) {
    *row = p->row;
    *col = p->col;
  } else {
    *row = 1;
    *col = 1;
  }
  stack_pop(stack);

  return c;
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
          fputc(' ', out);
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

        fputc(' ', out);
        fputc(' ', out);

        while ((c = fgetc(fp)) != EOF) {

          if (c == '\n') {
            fputc('\n', out);
            start_of_line = 1;
            prev = 0;
            continue;
          }

          fputc(' ', out);

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

  Token *tok = isIdentifier(fp, row, col, 0);
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

Token *isLogicalop(FILE *fp, int *row, int *col) {
  int start_row = *row;
  int start_col = *col;

  char c = nextChar(fp, row, col);
  char n = nextChar(fp, row, col);

  char buf[3] = {0};

  if ((c == '&' && n == '&') || (c == '|' && n == '|')) {
    buf[0] = c;
    buf[1] = n;
    return token_create(buf, start_row, start_col, -1, "LOGICAL");
  }

  ungetChar(n, fp, row, col);

  if (c == '&' || c == '|' || c == '!' || c == '^' || c == '~') {
    buf[0] = c;
    return token_create(buf, start_row, start_col, -1, "LOGICAL");
  }

  ungetChar(c, fp, row, col);
  return NULL;
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
  if (c != '+' && c != '-') {
    ungetChar(c, fp, row, col);
    return NULL;
  }

  char n = nextChar(fp, row, col);
  char buf[3];
  if (n == c) {
    buf[0] = c;
    buf[1] = n;
    buf[2] = '\0';
  } else {
    ungetChar(n, fp, row, col);
    buf[0] = c;
    buf[1] = '\0';
  }

  return token_create(buf, start_row, start_col, -1, "ADDOP");
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
  if (!stack)
    stack = stack_create();

  static int row = 1, col = 1;
  static int index = 0;

  char c;

  while ((c = nextChar(fp, &row, &col)) != EOF) {
    if (isspace(c))
      continue;
    break;
  }

  if (c == EOF) {
    return token_create("EOF", row, col, -1, "EOF");
    stack_destroy(stack);
  }

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
  if ((tok = isLogicalop(fp, &row, &col)))
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
