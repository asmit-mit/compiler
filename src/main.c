#include <stdio.h>

#include "compiler/compiler.h"

int main(int argc, char *argv[]) {
  // if (argc != 2) {
  //   printf("Use as ./compile <input-file-location>\n");
  //   return 0;
  // }

  // char *input_file = argv[1];
  char input_file[] = "main.c";
  compile(input_file);

  return 0;
}
