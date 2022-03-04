#include <stdlib.h>
#include <glob.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <fnmatch.h>
#include <stdbool.h>
#include "config.h"

static glob_t glob_rslt;
const int glob_flg = GLOB_APPEND;

static int run(char **cmd);

static int run_git_act(char *repo, int gitargc, char *gitargv[]);

static char **rd_gitignore();

static bool is_ignored(const char *repo, char **ign_lst);

static int dglob(char *root);
