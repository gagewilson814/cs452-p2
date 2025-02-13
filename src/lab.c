#include "lab.h"
#include <ctype.h>
#include <errno.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

void parse_args(int argc, char **argv) {
  int c;

  while ((c = getopt(argc, argv, "v")) != -1)
    switch (c) {
    case 'v':
      printf("shell version %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      exit(EXIT_SUCCESS);

    default:
      fprintf(stderr, "Invalid argument\n");
      exit(EXIT_FAILURE);
    }
}

void sh_destroy(struct shell *sh) {
  free(sh->prompt);
  free(sh);
  sh = NULL;
}

void sh_init(struct shell *sh) {

  if (sh == NULL) {
    fprintf(stderr, "The shell failed to initialize.\n");
    exit(EXIT_FAILURE);
  }
  /* See if we are running interactively.  */
  sh->shell_terminal = STDIN_FILENO;
  sh->shell_is_interactive = isatty(sh->shell_terminal);

  if (sh->shell_is_interactive) {
    /* Loop until we are in the foreground.  */
    while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
      kill(-sh->shell_pgid, SIGTTIN);

    /* Ignore interactive and job-control signals.  */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    /* Put ourselves in our own process group.  */
    sh->shell_pgid = getpid();
    if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Grab control of the terminal.  */
    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

    /* Save default terminal attributes for shell.  */
    tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
  }
}

bool do_builtin(struct shell *sh, char **argv) {
  UNUSED(sh);
  UNUSED(argv);
  return true;
}

char *trim_white(char *line) {
  if (line == NULL) {
    return NULL;
  }

  char *line_copy = strdup(line);
  if (line_copy == NULL) {
    fprintf(stderr, "trim_white: allocation error\n");
    exit(EXIT_FAILURE);
  }

  char *start = line_copy;
  while (isspace(*start)) {
    start++;
  }

  int end = strlen(start) - 1;
  while (end >= 0 && isspace(start[end])) {
    start[end] = '\0';
    end--;
  }

  char *trimmed_line = strdup(start);
  free(line_copy);
  line_copy = NULL;
  return trimmed_line;
}

void cmd_free(char **line) {
  if (line == NULL) {
    return;
  }
  for (int i = 0; line[i] != NULL; i++) {
    free(line[i]);
  }
  free(line);
}

char **cmd_parse(char const *line) {
  int max_args = sysconf(_SC_ARG_MAX);

  char **args = malloc((max_args + 1) * sizeof(char *));
  if (args == NULL) {
    fprintf(stderr, "cmd_parse: allocation error\n");
    exit(EXIT_FAILURE);
  }

  char *line_copy = strdup(line); // duplicate the line
  if (line_copy == NULL) {
    fprintf(stderr, "cmd_parse: allocation error\n");
    exit(EXIT_FAILURE);
  }

  int i = 0;
  char *token;
  char *saveptr;

  token = strtok_r(line_copy, " \t\r\n", &saveptr); // tokenize the line

  /* we will continue to tokenize the line until we reach the end of the line or
  we run out of arguments to pass in the execvp function */

  while (token != NULL && i < max_args) {
    args[i] = strdup(token); // duplicate the token and store it in args
    if (args[i] == NULL) {
      fprintf(stderr, "cmd_parse: allocation error\n");
      exit(EXIT_FAILURE);
    }
    i++;
    token = strtok_r(NULL, " \t\r\n", &saveptr); // get the next token
  }
  args[i] = NULL; // set the last argument to NULL

  free(line_copy);
  line_copy = NULL;
  return args;
}

int change_dir(char **dir) {
  UNUSED(dir);
  return 0;
}

char *get_prompt(const char *env) {
  UNUSED(env);
  return NULL;
}