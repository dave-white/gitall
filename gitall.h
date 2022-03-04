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
#include "config.h"

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

static glob_t glob_rslt;
const int glob_flg = GLOB_APPEND;

struct strary {
    int strc;
    char **strv;
} strary;

static char *run(char **argv);

static int run_git(char *repo, int gitargc, char *gitargv[]);

static int rd_gitignore();

static int dglob(char *root);
