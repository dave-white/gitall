#include "gitall.h"

static int run(char **cmd)
{
    if (fork() == 0) {
	execvp(cmd[0], cmd);
    } else {
	wait(NULL);
    }
    return 0;
}

static int run_git_act(char *repo, int gitargc, char *gitargv[])
{
    char **argv = (char **)calloc(4+gitargc, sizeof(char *));
    argv[0] = "git";
    argv[1] = "-C";
    argv[2] = repo;
    for (int argi=0; argi<gitargc; argi++) argv[3+argi] = gitargv[1+argi];
    argv[4 - 1 + gitargc] = NULL;
    printf("\n==== %s ", repo);
    int i = 0;
    for (int i=0; i < 79 - (strlen(repo) + 6); i++) printf("=");
    printf("\n");
    run(argv);
    free(argv);
    return 0;
}

static char **rd_gitignore()
{
    FILE *ignf = fopen(GITIGNOREF, "r");
    if (ignf == NULL) return NULL;
    char **ign_lst = (char **)malloc(sizeof(char *));
    char *ln;
    size_t n;
    ssize_t len;
    int i = 0;
    while ((len = getline(&ln, &n, ignf)) != -1) {
	i++;
	ign_lst = realloc(ign_lst, (i+1)*sizeof(char *));
	char *ptr = (char *)calloc(80, sizeof(char));
	ign_lst[i-1] = ptr;
	/* memset(ign_lst[i-1], 0, 80*sizeof(char)); */
	/* strcpy(ign_lst[i-1], "test"); */
	/* ign_lst[i-1] = "test"; */
	strncpy(ign_lst[i-1], ln, len-1);
    }
    printf("Ignoring: ");
    for (int j=0; j<i; j++) {
 	printf("%s, ", ign_lst[j]);
    }
    printf("\n");
    ign_lst[i] = NULL;
    fclose(ignf);
    return ign_lst;
}

static bool is_ignored(const char *repo, char **ign_lst)
{
    if (ign_lst == NULL) return false;
    int i = 0;
    while (ign_lst[i] != NULL) {
	if (fnmatch(ign_lst[i], repo, 0) == 0) {
	    return true;
	}
	i++;
    }
    return false;
}

static int dglob(char *root)
{
    char * const root_lst[] = {root, NULL};
    const int flg = FTS_NOSTAT|FTS_PHYSICAL|FTS_SEEDOT;
    FTS *tree = fts_open(root_lst, flg, NULL);
    FTSENT *fent;
    char *pat = (char *)calloc(8, sizeof(char));
    while ((fent = fts_read(tree))) {
	if (fent->fts_info == FTS_D) {
	    pat = realloc(pat, fent->fts_pathlen*sizeof(char)+40);
	    memset(pat, 0, fent->fts_pathlen*sizeof(char)+40);
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
    char **ign_lst = rd_gitignore();
    char *repo = (char *)calloc(80, sizeof(char));
    /* memset(repo, 0, 80*sizeof(char)); */
    int len = 0;
    for (int i=0; i<glob_rslt.gl_pathc; i++) {
	memset(repo, 0, (len+1)*sizeof(char));
	len = strlen(glob_rslt.gl_pathv[i]) - 5;
	repo = realloc(repo, sizeof(char)*(len+1));
	strncpy(repo, glob_rslt.gl_pathv[i], len);
	if (is_ignored(repo, ign_lst)) {
	    continue;
	} else {
	    run_git_act(repo, argc - 1, argv);
	}
    }
    free(repo);
    free(ign_lst);
    /* globfree(&glob_rslt); */
    return 0;
}
