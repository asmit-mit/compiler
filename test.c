#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  printf("Hello world!\n");
  int *temp = malloc(sizeof(int));
  *temp = 4;
  printf("ptr is pointing to value: %d\n", *temp);
  return 0;
}
