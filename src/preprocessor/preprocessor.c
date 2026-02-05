#include "preprocessor/preprocessor.h"

#include <ctype.h>
#include <stdio.h>

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
