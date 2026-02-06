#include <stdio.h>

int max(int a, int b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

int main() {
  printf("Hello World!");
  int a = 10, b = 20;
  printf("Max of a and b is: %d\n", max(a, b));
  return 0;
}
