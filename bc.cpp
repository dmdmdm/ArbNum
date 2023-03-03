// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#include <stdio.h>
#ifdef __linux__
#include <readline/history.h>
#include <readline/readline.h>
#endif
#include <string.h>
#include <time.h>

#include "arbnum.h"
#include "bc_calc.h"

static void usage() {
  fprintf(stderr,
          "Usage: bc\n"
          "\n"
          "bc - An arbitrary precision calculator language\n"
          "\n"
          "Enter 'help' once in the program for help\n"
          "\n"
          "Copyright (c) 2020 Dave McKellar\n");
  exit(1);
}

int main(const int argc, char* argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "-?") == 0) usage();

    if (strcmp(argv[1], "-t") == 0) {
      printf("Running tests\n");
      const time_t start = time(NULL);
      const bool result = ArbNum::testAll();
      const time_t duration = time(NULL) - start;
      printf("Took %ld seconds\n", (long)duration);
      exit(!result);
    }

    usage();
  }

#ifdef __linux__
  char* line;
#else
  char line[1024 * 5];
  char* p;
#endif

  Calc calc;
  for (;;) {
#ifdef __linux__
    if ((line = readline("bc> ")) == NULL) break;
    if (*line == '\0') continue;
    add_history(line);
#else
    printf("bc> ");
    if (fgets(line, sizeof(line), stdin) == NULL) break;
    if ((p = strchr(line, '\r')) != NULL) *p = '\0';
    if ((p = strchr(line, '\n')) != NULL) *p = '\0';
#endif

    const time_t start = time(NULL);

    calc.run(line);

#ifdef __linux__
    free(line);
#endif

    const time_t duration = time(NULL) - start;
    if (duration > 1) {
      printf("Took %ld seconds\n", (long)duration);
    }
  }
#ifdef __linux__
  putchar('\n');
#endif
}
