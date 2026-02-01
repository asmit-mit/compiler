#include "preprocessor/preprocessor.h"

#include <ctype.h>
#include <stdio.h>

void skipCommentsDirectivesAndStrings(FILE *fp) {
  FILE *out = fopen("temp.c", "w");
  if (!out) {
    perror("temp.c");
    return;
  }

  int c, next;
  int in_string = 0, in_char = 0;
  int start_of_line = 1;

  while ((c = fgetc(fp)) != EOF) {

    if (start_of_line) {
      int temp = c;

      while (isspace(temp) && temp != '\n') {
        fputc(temp, out);
        temp = fgetc(fp);
      }

      if (temp == '#') {
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '\n') {
            fputc('\n', out);
            break;
          }
          fputc(' ', out);
        }
        start_of_line = 1;
        continue;
      }

      ungetc(temp, fp);
      c = fgetc(fp);
      start_of_line = 0;
    }

    if (c == '"' && !in_char) {
      in_string = 1;
      fputc('"', out);

      while ((c = fgetc(fp)) != EOF) {
        if (c == '\\') {
          fputc(' ', out);
          fputc(' ', out);
          fgetc(fp);
          continue;
        }
        if (c == '"') {
          fputc('"', out);
          in_string = 0;
          break;
        }
        if (c == '\n')
          fputc('\n', out);
        else
          fputc(' ', out);
      }
      continue;
    }

    if (c == '\'' && !in_string) {
      in_char = 1;
      fputc('\'', out);

      while ((c = fgetc(fp)) != EOF) {
        if (c == '\\') {
          fputc(' ', out);
          fputc(' ', out);
          fgetc(fp);
          continue;
        }
        if (c == '\'') {
          fputc('\'', out);
          in_char = 0;
          break;
        }
        if (c == '\n')
          fputc('\n', out);
        else
          fputc(' ', out);
      }
      continue;
    }

    if (!in_string && !in_char && c == '/') {
      next = fgetc(fp);

      if (next == '/') {
        fputc(' ', out);
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '\n') {
            fputc('\n', out);
            break;
          }
          fputc(' ', out);
        }
        start_of_line = 1;
        continue;
      }

      if (next == '*') {
        fputc(' ', out);
        fputc(' ', out);
        while ((c = fgetc(fp)) != EOF) {
          if (c == '*' && (next = fgetc(fp)) == '/') {
            fputc(' ', out);
            fputc(' ', out);
            break;
          }
          if (c == '\n')
            fputc('\n', out);
          else
            fputc(' ', out);
        }
        continue;
      }

      ungetc(next, fp);
    }

    fputc(c, out);

    if (c == '\n')
      start_of_line = 1;
  }

  fclose(out);
}
