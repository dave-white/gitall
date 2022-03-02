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
    char *argv[4 + gitargc];
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
    return 0;
}

static char **rd_gitignore()
{
    FILE *ignf = fopen(GITIGNOREF, "r");
    if (ignf == NULL) return NULL;
    char **ign_lst = malloc(sizeof(char **));
    char *ign_ent;
    char *ln;
    size_t n;
    ssize_t len;
    int ign_cnt = 0;
    while ((len = getline(&ln, &n, ignf)) != -1) {
	ign_cnt++;
	ign_lst = realloc(ign_lst, sizeof(char **)*ign_cnt);
	ign_lst[ign_cnt - 1] = malloc(len*sizeof(char));
	strncpy(ign_lst[ign_cnt - 1], ln, len - 1);
    }
    printf("Ignoring: ");
    for (int i=0; i<ign_cnt; i++) {
	printf("%s, ", ign_lst[i]);
    }
    printf("\n");
    /* if (ln) free(*ln); */
    ign_lst[ign_cnt] = NULL;
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
    while ((fent = fts_read(tree))) {
	if (fent->fts_info == FTS_D) {
	    char glob_pat[fent->fts_pathlen + 5 + 1];
	    memset(glob_pat, '\0', sizeof(glob_pat));
	    strcpy(glob_pat, fent->fts_path);
	    strcat(glob_pat, "/.git");
	    glob(glob_pat, glob_flg, NULL, &glob_rslt);
	} else {
	    continue;
	}
    }
    fts_close(tree);
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

    printf("Running `git %s` on local repos.\n", argv[1]);
    char **ign_lst = rd_gitignore();
    char *repo = malloc(1);
    int len = 1;
    for (int i=0; i<glob_rslt.gl_pathc; i++) {
	memset(repo, '\0', len);
	len = strlen(glob_rslt.gl_pathv[i]) - 5;
	repo = realloc(repo, sizeof(char)*len);
	strncpy(repo, glob_rslt.gl_pathv[i], len);
	if (is_ignored(repo, ign_lst)) {
	    continue;
	} else {
	    run_git_act(repo, argc - 1, argv);
	}
    }
    free(repo);
    free(ign_lst);
    globfree(&glob_rslt);
    return 0;
}
