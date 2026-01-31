#include "lexer/lexer.h"

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

void skipDirective(FILE *fp, int *row, int *col) {
  char c;
  while ((c = nextChar(fp, row, col)) != EOF) {
    if (c == '\n')
      break;
  }
}

// int skipComment(FILE *fp, int *row, int *col) {
//   char c = nextChar(fp, row, col);
//   char n = nextChar(fp, row, col);
//
//   if (c != '/' || (n != '/' && n != '*')) {
//     ungetChar(n, fp, row, col);
//     ungetChar(c, fp, row, col);
//     return 0;
//   }
//
//   if (n == '/') {
//     while ((c = nextChar(fp, row, col)) != EOF) {
//       if (c == '\n')
//         break;
//     }
//
//     return 1;
//   }
//
//   if (n == '*') {
//     char prev = 0;
//     while ((c = nextChar(fp, row, col)) != EOF) {
//       if (prev == '*' && c == '/')
//         break;
//       prev = c;
//     }
//   }
//
//   return 1;
// }

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
      "unsigned", "void",     "volatile", "while"};

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
    if (c == '#') {
      skipDirective(fp, &row, &col);
      continue;
    }
    // if (c == '/') {
    //   ungetChar(c, fp, &row, &col);
    //   if (skipComment(fp, &row, &col))
    //     continue;
    // }
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
