#include <stdio.h>

#include "compiler/compiler.h"
#include "preprocessor/preprocessor.h"

#include "lexer/lexer.h"
#include "lexer/token.h"

void compile(const char *input_file) {
  FILE *fp = fopen(input_file, "r");
  skipCommentsDirectivesAndStrings(fp);
  fclose(fp);

  FILE *temp_fp = fopen("temp.c", "r");

  Token *tok, *prev_tok;
  do {
    if (tok && strcmp(tok->type, "KEYWORD") == 0) 
      prev_tok = tok;
    tok = getNextToken(temp_fp);
    if (tok == NULL) {
      fprintf(stderr, "Lexer error\n");
      break;
    }

    if (strcmp(tok->type, "IDENTIFIER") == 0) {
      printf("<%s, %d, %d, %d, %s>\n", prev_tok->token_name, prev_tok->row,
             prev_tok->col, prev_tok->index, prev_tok->type);
      printf("<%s, %d, %d, %d, %s>\n\n", tok->token_name, tok->row, tok->col,
             tok->index, tok->type);
    }

  } while (strcmp(tok->type, "EOF") != 0);

  fclose(temp_fp);
}
