#include "lexer/token.h"

Token *getToken(char token_name[], int row, int col, int index, char type[]) {
  Token* tk = malloc(sizeof(Token));
  tk->row = row;
  tk->col = col;
  tk->index = index;
  strcpy(tk->token_name, token_name);
  strcpy(tk->type, type);
  return tk;
}
