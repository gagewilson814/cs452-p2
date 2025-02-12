#include "lab.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <asm-generic/signal.h>

void parse_args(int argc, char **argv) {
  int c;

  while ((c = getopt(argc, argv, "v")) != -1)
    switch (c) {
    case 'v':
      printf("lab version %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      exit(0);
    }
}

void sh_destroy(struct shell *sh) {
  free(sh->prompt);
  free(sh);
  sh = NULL;
}

void sh_init(struct shell *sh) {
  /* See if we are running interactively.  */
  sh->shell_terminal = STDIN_FILENO;
  sh->shell_is_interactive = isatty (sh->shell_terminal);

  if (sh->shell_is_interactive)
    {
      /* Loop until we are in the foreground.  */
      while (tcgetpgrp (sh->shell_terminal) != (sh->shell_pgid = getpgrp ()))
        kill (- sh->shell_pgid, SIGTTIN);

      /* Ignore interactive and job-control signals.  */
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGCHLD, SIG_IGN);

      /* Put ourselves in our own process group.  */
      sh->shell_pgid = getpid ();
      if (setpgid (sh->shell_pgid, sh->shell_pgid) < 0)
        {
          perror ("Couldn't put the shell in its own process group");
          exit (1);
        }

      /* Grab control of the terminal.  */
      tcsetpgrp (sh->shell_terminal, sh->shell_pgid);

      /* Save default terminal attributes for shell.  */
      tcgetattr (sh->shell_terminal, &sh->shell_tmodes);
    }
}

bool do_builtin(struct shell *sh, char **argv) {
  UNUSED(sh);
  UNUSED(argv);
  return true;
}

char *trim_white(char *line) {
  UNUSED(line);
  return NULL;
}

void cmd_free(char **line) {
  for (int i = 0; line[i] != NULL; i++) {
    free(line[i]);
  }
  free(line);
}

char **cmd_parse(char const *line) {
  UNUSED(line);
  return NULL;
}

int change_dir(char **dir) {
  UNUSED(dir);
  return 0;
}

char *get_prompt(const char *env) {
  UNUSED(env);
  return NULL;
}