#include "lab.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void parse_args(int argc, char **argv) {}

void sh_destroy(struct shell *sh) {}

void sh_init(struct shell *sh) {}

bool do_builtin(struct shell *sh, char **argv) {}

char *trim_white(char *line) {}

void cmd_free(char **line) {}

char **cmd_parse(char const *line) {}

int change_dir(char **dir) {}

char *get_prompt(const char *env) {}