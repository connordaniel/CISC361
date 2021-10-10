
#include "get_path.h"
#include <glob.h>

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void execCommand(char *command, char **args, int status);
void printExec(char * command);
int findWildcard(char wc, char **args);
void cleanup();
void runGlob(int index, char *commandpath, struct pathelement *pathlist, char **args, glob_t globIn, int status);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXIN 128