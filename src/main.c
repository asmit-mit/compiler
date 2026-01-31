#include <stdio.h>

#include "lexer/lexer.h"
#include "lexer/token.h"

#include "hashmap/hashmap.h"
#include "hashmap/symbol.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Use as ./compile <input-file-locaition>\n");
    return 0;
  }

  char *input_file = argv[1];
  FILE *fp = fopen(input_file, "r");

  Token *tok;
  do {
    tok = getNextToken(fp);
    if (tok == NULL) {
      fprintf(stderr, "Lexer error\n");
      break;
    }

    printf("<%s, %d, %d, %d, %s>\n", tok->token_name, tok->row, tok->col,
           tok->index, tok->type);

  } while (strcmp(tok->type, "EOF") != 0);

  fclose(fp);
  
  return 0;
}
