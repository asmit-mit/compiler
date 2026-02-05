#include <stdio.h>

#include "compiler/compiler.h"
#include "preprocessor/preprocessor.h"

#include "lexer/lexer.h"
#include "lexer/symbol.h"
#include "lexer/token.h"

#include "hashmap.h"

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
      printf("lexeme: %-15s | size: %-4d | type: %-12s | scope: %-8s\n",
             s->lexeme, s->size, s->type, s->scope);
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

  HashMap *map = hashmap_create(5, symbol_getIndex, symbol_compare);

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
      if (curr->row == last_type_row) {
        Token *peek = getNextToken(temp_fp);

        if (peek && strcmp(peek->type, "PUNCT") == 0 &&
            peek->token_name[0] == '(') {

          hashmap_insert(
              map, symbol_create(curr->token_name, -1, "function", "global"));

          prev = peek;
          curr = getNextToken(temp_fp);
          continue;
        }

        hashmap_insert(map,
                       symbol_create(curr->token_name, isTypeKeyword(last_type),
                                     last_type, "global"));
      } else if (isBuiltin(curr->token_name)) {
        hashmap_insert(
            map, symbol_create(curr->token_name, -1, "not in file", "global"));
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
