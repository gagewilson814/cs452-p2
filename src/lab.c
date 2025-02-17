#include "lab.h"
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  sh->prompt = NULL;
}

void sh_init(struct shell *sh) {

  if (sh == NULL) {
    fprintf(stderr, "The shell failed to initialize.\n");
    exit(EXIT_FAILURE);
  }

  sh->prompt = get_prompt("MY_PROMPT");
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

  if (sh == NULL || argv == NULL) {
    fprintf(stderr, "something went wrong\n");
    return false;
  }

  bool rval = false;
  if (strcmp(argv[0], "exit") == 0) {
    rval = false;
    sh_destroy(sh);
    exit(EXIT_SUCCESS);
  }
  if (strcmp(argv[0], "cd") == 0) {
    rval = true;
    change_dir(argv);
  }
  if (strcmp(argv[0], "history") == 0) {
    rval = false;
    HIST_ENTRY **hist_list = history_list();
    if (hist_list) {
      for (int i = 0; hist_list[i]; i++) {
        printf("%d: %s\n", i + history_base, hist_list[i]->line);
      }
    }
  }
  return rval;
}

char *trim_white(char *line) {
  if (line == NULL) {
    return NULL;
  }

  // lets make a copy to prevent modifying OG string
  char *line_copy = strdup(line);
  if (line_copy == NULL) {
    fprintf(stderr, "trim_white: allocation error\n");
    exit(EXIT_FAILURE);
  }

  // trim spaces from the front of the sting
  char *start = line_copy;
  while (isspace(*start)) {
    start++;
  }

  // trim spaces from the back end of the string
  int end = strlen(start) - 1;
  while (end >= 0 && isspace(start[end])) {
    start[end] = '\0'; // replace the space with a null terminator
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
  line = NULL;
}

char **cmd_parse(char const *line) {

  if (line == NULL) {
    return NULL;
  }

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
  if (dir[1] == NULL) {
    char *home = getenv("HOME");
    if (home == NULL) {
      struct passwd *pw = getpwuid(getuid());
      if (pw == NULL) {
        perror("change_dir");
        return -1;
      }
      home = pw->pw_dir;
    }
    if (chdir(home) != 0) {
      perror("change_dir");
      return -1;
    }
  } else {
    if (chdir(dir[1]) != 0) {
      perror("change_dir");
      return -1;
    }
  }
  return 0;
}

char *get_prompt(const char *env) {
  char *env_prompt = getenv(env);
  char *prompt;
  if (env_prompt == NULL) {
    prompt = strdup("shell>");
  } else {
    prompt = strdup(env_prompt);
  }

  if (prompt == NULL) {
    fprintf(stderr, "get_prompt: allocation error\n");
    exit(EXIT_FAILURE);
  }

  return prompt;
}