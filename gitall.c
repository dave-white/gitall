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

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("No git operation given.\n");
    return 0;
  }

  char *home = getenv("HOME");
  printf("Globbing for local git repos in %s.\n", home);
  char * const root_lst[] = {home, NULL};
  const int fts_flg = FTS_NOSTAT|FTS_PHYSICAL|FTS_SEEDOT;
  FTS *ftree = fts_open(root_lst, fts_flg, NULL);
  FTSENT *fnode;
  int pat_szmult = 1;
  char *pat = (char *)calloc(128, sizeof(char));
  while ((fnode = fts_read(ftree))) {
    if (fnode->fts_info == FTS_D) {
      if (fnode->fts_pathlen+5 > 128*pat_szmult - 8) {
	pat_szmult++;
	pat = realloc(pat, 128*pat_szmult*sizeof(char));
      }
      memset(pat, 0, fnode->fts_pathlen+5+8);
      strncpy(pat, fnode->fts_path, fnode->fts_pathlen);
      strcat(pat, "/.git");
      glob(pat, glob_flg, NULL, &glob_rslt);
    } else {
      continue;
    }
  }
  fts_close(ftree);
  free(pat);
  if (glob_rslt.gl_pathc < 1) {
    printf("No repos globbed.\n");
    return 0;
  }

  printf("Running `git %s` on local repos.\n", argv[1]);

  // Read the ignore file.
  struct strary igns;
  igns.strc = 0;
  // igns.strv = NULL;
  if (strcmp(argv[1], "pull") != 0) {
    FILE *ignf = fopen(GITIGNOREF, "r");
    if (ignf != NULL) {
      igns.strv = (char **)calloc(128, sizeof(char *));
      int ignv_szmult = 1;
      char *line = (char *)calloc(128, sizeof(char));
      size_t n;
      ssize_t linelen;
      char *ptr;
      while ((linelen = getline(&line, &n, ignf)) != -1) {
	ptr = (char *)calloc(linelen, sizeof(char));
	strncpy(ptr, line, linelen-1);
	igns.strv[igns.strc] = ptr;
	igns.strc++;
	if (igns.strc > 128*ignv_szmult - 8) {
	  ignv_szmult++;
	  igns.strv = realloc(igns.strv, 128*ignv_szmult*sizeof(char *));
	}
      }
      fclose(ignf);
    }
    if (igns.strc > 0) {
      printf("Ignoring: %s", igns.strv[0]);
      for (int i=1; i<igns.strc; i++) printf(", %s", igns.strv[i]);
      printf("\n");
    }
  }

  char **gitargv = (char **)calloc(4+(argc-1), sizeof(char *));
  gitargv[0] = "git";
  gitargv[1] = "-C";
  for (int i=0; i<(argc-1); i++)
    gitargv[3+i] = argv[1+i];
  char *repo = (char *)calloc(128, sizeof(char));
  int repo_szmult = 1;
  int len = 0;
  int skip;
  FILE *output;
  int tuyau[2]; // "tuyau" is, roughly, French for "pipe"
  pid_t pid;
  for (int i=0; i<glob_rslt.gl_pathc; i++) {
    len = strlen(glob_rslt.gl_pathv[i]) - 5;
    if (len > 128*repo_szmult - 8) {
      repo_szmult++;
      repo = realloc(repo, 128*repo_szmult*sizeof(char));
    }
    memset(repo, 0, (len+1)*sizeof(char));
    strncpy(repo, glob_rslt.gl_pathv[i], len);
    skip = 0;
    for (int j=0; j<igns.strc; j++) {
      if (fnmatch(igns.strv[j], repo, 0) == 0) {
	skip = 1;
	break;
      }
    }
    if (skip)
      continue;

    gitargv[2] = repo;

    if(pipe(tuyau))
      die("tuyau failed.\n");

    pid = fork();
    if (pid < (pid_t) 0) {
      die("Fork failed.\n");
    } else if (pid == 0) {
      // Child
      close(tuyau[0]); // Close readable end.
      dup2 (tuyau[1], STDOUT_FILENO);
      dup2 (tuyau[1], STDERR_FILENO);
      close(tuyau[1]);
      execvp(gitargv[0], gitargv);
      die("Spawned execvp going down.\n");
    } else {
      // Parent
      close(tuyau[1]); // Close writable end.
      output = fdopen(tuyau[0], "r");
      int c;
      if ((c = fgetc(output)) != EOF) {
        printf("\n==== %s ", repo);
        for (int i=0; i < 79 - (strlen(repo) + 6); i++) printf("=");
        printf("\n");
        putchar(c);
        while ((c = fgetc(output)) != EOF)
      putchar(c);
      }
      fclose(output);
      wait(NULL);
    }
  }

  free(gitargv);
  free(repo);
  for (int i=0; i<igns.strc; i++)
    free(igns.strv[i]);
  globfree(&glob_rslt);
  return 0;
}
