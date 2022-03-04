#include "gitall.h"

static char *run(char **argv)
{
  int link[2];
  pid_t pid;
  char *out = (char *)calloc(4096, sizeof(char));

  if (pipe(link)==-1) die("pipe");

  if ((pid = fork()) == -1) die("fork");

  if(pid == 0) {
    dup2 (link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    execvp(argv[0], argv);
    die("execvp");
  } else {
    close(link[1]);
    int nbytes = read(link[0], out, 4095*sizeof(char));
    wait(NULL);
  }
  return out;
}

static int run_git(char *repo, int gitargc, char *gitargv[])
{
  char **argv = (char **)calloc(4+gitargc, sizeof(char *));
  argv[0] = "git";
  argv[1] = "-C";
  argv[2] = repo;
  for (int argi=0; argi<gitargc; argi++) argv[3+argi] = gitargv[1+argi];
  argv[4 - 1 + gitargc] = NULL;

  char *out = run(argv);

  if (strlen(out) > 0) {
    printf("\n==== %s ", repo);
    int i = 0;
    for (int i=0; i < 79 - (strlen(repo) + 6); i++) printf("=");
    printf("\n");
    printf("%s\n", out);
  }

  free(out);
  free(argv);
  return 0;
}

static int rd_gitignore(struct strary *igns)
{
  igns->strc = 0;
  igns->strv = NULL;

  FILE *ignf = fopen(GITIGNOREF, "r");
  if (ignf == NULL) return 1;
  char *ln;
  size_t n;
  ssize_t len;
  igns->strv = (char **)calloc(127, sizeof(char *));
  int i = 0;
  while ((len = getline(&ln, &n, ignf)) != -1) {
    char *ptr = (char *)calloc(127, sizeof(char));
    strncpy(ptr, ln, len-1);
    igns->strv[i] = ptr;
    i++;
    //ign_lst = realloc(ign_lst, (i+1)*sizeof(char *));
    /* memset(ign_lst[i-1], 0, 80*sizeof(char)); */
    /* strcpy(ign_lst[i-1], "test"); */
    /* ign_lst[i-1] = "test"; */
  }
  igns->strc = i;
  fclose(ignf);
  return 0;
}

static int dglob(char *root)
{
  char * const root_lst[] = {root, NULL};
  const int flg = FTS_NOSTAT|FTS_PHYSICAL|FTS_SEEDOT;
  FTS *tree = fts_open(root_lst, flg, NULL);
  FTSENT *fent;
  char *pat = (char *)calloc(511, sizeof(char));
  while ((fent = fts_read(tree))) {
    if (fent->fts_info == FTS_D) {
      //pat = realloc(pat, fent->fts_pathlen*sizeof(char)+40);
      //memset(pat, 0, fent->fts_pathlen*sizeof(char)+40);
      memset(pat, 0, 511*sizeof(char));
      strncpy(pat, fent->fts_path, fent->fts_pathlen);
      strcat(pat, "/.git");
      glob(pat, glob_flg, NULL, &glob_rslt);
    } else {
      continue;
    }
  }
  fts_close(tree);
  free(pat);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("No git operation given.\n");
    return 0;
  }

  char *home = getenv("HOME");
  printf("Globbing for local git repos in %s.\n", home);
  dglob(home);
  if (&glob_rslt == NULL || glob_rslt.gl_pathc == 0) {
    printf("No repos globbed.\n");
    return 0;
  }

  printf("Running `git %s` on local repos.\n", argv[1]);

  struct strary igns;
  if (strcmp(argv[1], "pull") == 0) {
      igns.strc = 0;
      igns.strv = NULL;
  } else {
      rd_gitignore(&igns);
      if (igns.strc > 0) {
	  printf("Ignoring: %s", igns.strv[0]);
	  for (int i=1; i<igns.strc; i++) printf(", %s", igns.strv[i]);
	  printf("\n");
      }
  }

  char *repo = (char *)calloc(80, sizeof(char));
  int len = 0;
  int skip;
  for (int i=0; i<glob_rslt.gl_pathc; i++) {
    memset(repo, 0, 80*sizeof(char));
    len = strlen(glob_rslt.gl_pathv[i]) - 5;
    //repo = realloc(repo, sizeof(char)*(len+1));
    strncpy(repo, glob_rslt.gl_pathv[i], len);
    skip = 0;
    for (int j=0; j<igns.strc; j++) {
      if (fnmatch(igns.strv[j], repo, 0) == 0) {
	skip = 1;
	break;
      }
    }
    if (skip) continue;
    run_git(repo, argc - 1, argv);
  }
  free(repo);
  for (int i=0; i<igns.strc; i++) free(igns.strv[i]);
  globfree(&glob_rslt);
  return 0;
}
