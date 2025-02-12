#include "lab.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void parse_args(int argc, char **argv) {
  int c;

  while ((c = getopt(argc, argv, "v")) != -1)
    switch (c) {
    case 'v':
      printf("lab version %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      break;
    }
}

void sh_destroy(struct shell *sh) {
  free(sh->prompt);
  free(sh->shell_terminal);
  free(sh->shell_is_interactive);
  free(sh->shell_terminal);
  free(sh->shell_pgid);
  free(sh);
  sh = NULL;
}

// void sh_init(struct shell *sh) {}

// bool do_builtin(struct shell *sh, char **argv) {}

// char *trim_white(char *line) {}

void cmd_free(char **line) {
  for (int i = 0; line[i] != NULL; i++) {
    free(line[i]);
  }
  free(line);
}

// char **cmd_parse(char const *line) {}

// int change_dir(char **dir) {}

// char *get_prompt(const char *env) {}