#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <glob.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char buffer [MAXIN];
  char promptBuff[PROMPTMAX];
  char **enviro;
  glob_t globIn;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while ( go )
  {
    /* print your prompt */
    printf("\n%s [%s]> ", prompt, pwd);
    /* get command line and process */
    if (fgets(buffer, MAXIN, stdin) != NULL) {
      int len = strlen(buffer);
      if(buffer[len - 1] == '\n')
        buffer[len-1] = 0;
      strcpy(commandline, buffer);
    }

    int i = 0;
    char *tok = strtok(commandline, " ");
    command = tok;
    memset(args, '\0', MAXARGS*sizeof(char*));
    while (tok) {
      args[i] = tok;
      tok = strtok(NULL, " ");
      i++;
    }

    if (command != NULL) {
        /* check for each built in command and implement */
        if (strcmp(command, "exit") == 0) {
          printExec(command);
          break;
        }

        //absolute path
        else if((command[0] == '/') || (command[0] == '.') && (command[1] == '/') || ((command[1] == '.') && (command[2] == '/'))) {
          if (access(command, X_OK) == -1) {
            printf("Cannot access %s", command);
            perror("Error");
          } else {
            printf("\n path %s\n", command);
            execCommand(command, args, status);
          }
        }
        //which
        else if (strcmp(command, "which") == 0) {
          for (int i = 1; args[i] != NULL; i++) {
            commandpath = which(args[i], pathlist);
            printf("\n%s", commandpath);
            free(commandpath);
          }
        }
        //where
        else if (strcmp(command, "where") == 0) {
          for (int i = 1; args[i] != NULL; i++) {
            commandpath = where(args[i], pathlist);
            free(commandpath);
          }
        }
        //cd
        else if (strcmp(command, "cd") == 0) {
          printExec(command);
          if (args[1] == NULL) {
            strcpy(owd, pwd);
            strcpy(pwd, homedir);
            chdir(pwd);
          } else if (strcmp(args[1], "-") == 0){
            p = pwd;
            pwd = owd;
            owd = p;
            chdir(pwd);
          } else if (args[1] != NULL && args[2] == NULL) {
            if (chdir(args[1]) == -1)
              perror("Error: ");
            else {
              memset(owd, '\0', strlen(owd));
              memcpy(owd, pwd, strlen(pwd));
              getcwd(pwd, PATH_MAX + 1);
            }
          }
        }

        //built-in list
        else if (strcmp(command, "list") == 0) {
          printExec(command);
          if (args[1] == NULL)
            list(pwd);
          else {
            int i = 1;
            while (args[i]) {
              if (access(args[i], X_OK) == -1)
                perror("\n Error: ");
              else {
                printf("\n%s\n", args[i]);
                list(args[i]);
              }
              i++;
            }
          }
        }

        //prompt
        else if (strcmp(command, "prompt") == 0) {
          printExec(command);
          if (args[1] == NULL) {
            printf("\n Input Prompt Prefix: ");
            if (fgets(promptBuff, PROMPTMAX, stdin) != NULL) {
              int len = strlen(promptBuff);
              if (promptBuff[len - 1] == '\n')
                promptBuff[len - 1] = 0;
              strtok(promptBuff, " ");
              strcpy(prompt, promptBuff);
            }
          }
          else strcpy(prompt, args[1]);
        }

        //printenv
        else if (strcmp(command, "printenv") == 0) {
          printExec(command);
          if (args[1] == NULL)
            printenv(enviro);
          else if (args[2] == NULL)
            printf("\n%s\n", getenv(args[1]));
          else
            printf("\nprintenv: Too Many Arguments\n");
        } 

        //setenv
        else if (strcmp(command, "setenv") == 0) {
          printExec(command);
          if (args[1] == NULL)
            printenv(enviro);
          else if(args[2] == NULL && (strcmp(args[1], "PATH") == 0 || strcmp(args[1], "HOME") == 0))
            printf("\nDo not set PATH or HOME to empty\n");
          else if (args[2] == NULL) {
            if (setenv(args[1], "", 1) == -1)
              perror("Error: ");
          } else if (args[3] == NULL) {
            if (setenv(args[1], args[2], 1) == -1)
              perror("Error: ");
            else {
              if (strcmp(args[1], "PATH") == 0) {
                deletepath(&pathlist);
                pathlist = NULL;
              } 
              if (strcmp(args[1], "HOME") == 0)
                homedir = args[2];
            }
          } else printf("\nError setenv: Too Many Arguments\n");
        }

        //pid
        else if (strcmp(command, "pid") == 0) {
          printExec(command);
          printf("\nPID: %d\n", getpid());
        }
        //pwd
        else if (strcmp(command, "pwd") == 0) {
          printExec(command);
          printf("\n%s\n", pwd);
        }

        //kill 
        else if (strcmp(command, "kill") == 0) {
          if (args[1] == NULL)
            printf("\nNo Argument Given for %s", command);
          else if (args[2] == NULL) {
            int temp = -1;
            sscanf(args[1], "%d", &temp);
            if (temp != -1) {
              if (kill(temp, 15) == -1)
                perror("Error: ");
            } else 
                printf("\nInvalid PID");
          } else if (args[3] = NULL) {
            int sig = 0;
            int temp = -1;
            sscanf(args[1], "%d", &sig);
            sscanf(args[2], "%d", &sig);
            if (sig < 0 && temp != - 1) {
              if (sig == -1 && temp == getpid()) {
                deletepath(&pathlist);
                free(args);
                free(commandline);
                free(owd);
                free(prompt);
                free(pwd);
                pathlist = NULL;
              }
              if (kill (temp, abs(sig)) == -1)
                perror("Error: ");
              
            } else 
                printf("\n Invalid arguments for %s", command);
          }
        } else {
            printf("Executed %s", command);
            int flagA = findWildcard('?', args);
            int flagB = findWildcard('*', args);

            if (strcmp(command, "ls") == 0 && flagA != -1)
              runGlob(flagA, commandpath, pathlist, args, globIn, status);
            else if (strcmp(command, "ls") == 0 && flagB != -1)
              runGlob(flagB, commandpath, pathlist, args, globIn, status);
            else {
              commandpath = which(command, pathlist);
              execCommand(commandpath, args, status);
              free(commandpath);
            }
        }
    }//finally
  }

  deletepath(&pathlist);
  free(args);
  free(commandline);
  free(owd);
  free(prompt);
  free(pwd);
  pathlist = NULL;
  exit(0);
  return 0;
} /* sh() */
/* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
char *which(char *command, struct pathelement *pathlist )
{
  char buffer[MAXIN];
  while (pathlist) {
    snprintf(buffer, MAXIN, "%s/%s", pathlist->element, command);

    if (access(buffer, X_OK) == -1)
      pathlist = pathlist->next;
    else {
      int len = strlen(buffer);
      char *ret = calloc(len+1, sizeof(char));
      strncpy(ret, buffer, len);
      return ret;
    }
  }
  return NULL;
} /* which() */
  /* similarly loop through finding all locations of command */
char *where(char *command, struct pathelement *pathlist )
{
  char buffer[MAXIN];
  char *ret;
  int flag = 0;
  
  while (pathlist) {
    snprintf(buffer, MAXIN, "%s/%s", pathlist->element, command);
    if (access(buffer, X_OK) == -1)
      pathlist = pathlist->next;
    else if (access(buffer, X_OK) != -1 && flag == 0) {
      flag = 1;
      int len = strlen(buffer);
      ret = calloc(len+1, sizeof(char));
      strncpy(ret, buffer, len);
      printf("\n%s", ret);
      pathlist = pathlist->next;
    } else if (access(buffer, X_OK) != -1) {
      printf("\n%s", buffer);
      pathlist = pathlist->next;
    }
  }
  return ret;
} /* where() */

 /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
void list ( char *dir )
{
  DIR *ndir = opendir(dir);
  struct dirent *file;
  if (ndir) {
    while ((file = readdir(ndir)) != NULL) 
      printf("%s\n", file->d_name);
  }
  closedir(ndir);
} /* list() */

//forks and execs to make new process
void execCommand(char *command, char **args, int status){
  if (command == NULL)
    fprintf(stderr, "%s: Command not found\n", args[0]);
  else {
    pid = fork();
    if (pid == 0) {
      execve(command, args, NULL);
      exit(EXIT_FAILURE);
    } else if (pid > 0) {
      do
        waitpid(pid, &status, WUNTRACED);
      while (!WIFEXITED(status) && !WIFSIGNALED(status)); 
    }
  }
}/* execCommand() */

//prints all env variables
void printenv(char ** envp) {
  int i = 0;
  while (envp[i] != NULL) {
    printf("%s\n", envp[i]);
    i++;
  }
}

void printExec(char * command) {
  printf("Executing %s", command);
}

//helper method to find wildcard character
//returns index of wildcard if there, -1 otherwise
int findWildcard(char wc, char **args) {
  int i = 0;
  char *j;
  while (args[i]) {
    j = strchr(args[i], wc);
    if (j != NULL)
      return i;
    i++;
  }
  return -1;
}

//takes in wildcard and computes glob expansion
//self-cleaning
void runGlob(int index, char *commandpath, struct pathelement *pathlist, char **args, glob_t globIn, int status) {
  globIn.gl_offs = index;
  glob(args[index], GLOB_DOOFFS, NULL, &globIn);
  for (int i = 0; i < index; i++) {
    globIn.gl_pathv[i] = calloc(sizeof(char), strlen(args[i]) + 1);
    strcpy(globIn.gl_pathv[i], args[i]);
  }
  commandpath = which(globIn.gl_pathv[0], pathlist);
  execCommand(commandpath, globIn.gl_pathv, status);
  free(commandpath);
  for (int i = 0; i < index; i++)
    free(globIn.gl_pathv[i]);
  globfree(&globIn);
}