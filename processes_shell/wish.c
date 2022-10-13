#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int runCmd(char *cbin, char *toks[128], char *rdout);
int findPos(char *toks[128], char *tok);
char * findCbin(char *paths[20], char *cmd);
char ** ltoks(char* line);
char ** lineToks(FILE* f);
int getTokens(char* line, char *toks[128]);
char ** strarr(int lna, int lns);
char * mkstr(int ln);

char *EXITCMD = "exit";
char *CDCMD = "cd";
char *PATHCMD = "path";
char *IFCMD = "if";
char ERRMSG[30] = "An error has occurred\n";

int main (int argc, char* argv[]) {
    int batch = argc > 1? 1: 0;
    FILE *f = batch == 1? fopen(argv[1], "r"): stdin;
    char **paths = strarr(50, 100);
    strcpy(paths[0], "/bin");
    paths[1] = NULL;

    while(1) {
        if (batch == 0)
            printf("wish> ");
        
        size_t szl = 1000;
        char *line = mkstr(szl);
        if(getline(&line, &szl, f) == EOF)
            exit(0);
        char * rp = strchr(line, '>');
        char * rdout = mkstr(100);
        strcpy(rdout, "");
        
        if (rp != NULL) {
            strcpy(rdout, rp + 1);
            rdout = strtok(rdout, " \n\t\r\f\v");
            if (rdout == NULL)
                write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
            rp[0] = '\0';
        }

        char **toks = ltoks(line);
        // printf("toks are: ");
        // for (int i = 0; toks[i] != NULL; i++)
        //     printf("%s ", toks[i]);
        // printf("\n");

        if (toks[0] == NULL || strcmp(toks[0], "") == 0) {
            continue;
        } else if(strcmp(toks[0], EXITCMD) == 0) {
            if (toks[1] != NULL)
                write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
            else
                exit(0);
        } else if(strcmp(toks[0], CDCMD) == 0) {
            if (toks[1] == NULL || toks[2] != NULL)
                write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
            else
                chdir(toks[1]);
        } else if(strcmp(toks[0], PATHCMD) == 0) {
            int j = 0;   
            for (j = 0; toks[j+1] != NULL; j++) {
                strcpy(paths[j], toks[j+1]);
            }
            paths[j] = NULL;
        } else if (strcmp(toks[0], IFCMD) == 0) { 
            int tnpos = findPos(toks, "then");

            if (toks[findPos(toks, "fi")+1] != NULL || tnpos == -1
                || (strcmp(toks[tnpos - 2], "==") != 0) && (strcmp(toks[tnpos - 2], "!=") != 0))
                write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
            else {
                char **ftoks = strarr(128, 100);
                char **stoks = strarr(128, 100);
                int i;
                for (i = 0; i < (tnpos + 1 - 4); i++) {
                    strcpy(ftoks[i], toks[i+1]);
                }
                ftoks[i] = NULL;

                for (i = 0; toks[tnpos+1+i] != NULL; i++) {
                    strcpy(stoks[i], toks[tnpos+1+i]);
                }
                stoks[i-1] = NULL;
                
                char *cbin = findCbin(paths, ftoks[0]);
                int ret = runCmd(cbin, ftoks, "");

                if ((strcmp(toks[tnpos - 2], "==") == 0 && ret == atoi(toks[tnpos - 1])) 
                    || (strcmp(toks[tnpos - 2], "!=") == 0 && ret != atoi(toks[tnpos - 1]))) {
                    char *cbin = findCbin(paths, stoks[0]);
                    runCmd(cbin, stoks, "");
                }
            }
        } else {
            char *cbin = findCbin(paths, toks[0]);
            if (strcmp(cbin, "") != 0) {
                runCmd(cbin, toks, rdout);
            }
            else 
                write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
        }

        // printf("paths are: ");
        // for (int i = 0; paths[i] != NULL; i++)
        //     printf("%s ", paths[i]);
        // printf("\n");
    }
}

// Run a command from its binary path and array of arguments
int runCmd(char *cbin, char *toks[128], char *rdout) {
    int st = -100;
    pid_t pid = fork();
    if (pid < 0) {
        write(STDERR_FILENO, ERRMSG, strlen(ERRMSG)); 
        exit(1);
    } else if (pid == 0) {
        int fd;
        if(strcmp(rdout, "") != 0) {
            fd = open(rdout, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execv(cbin, toks);

    } else {
        wait(&st);
    }
    return st;
}

// Find index of an element in an array of strings
int findPos(char *toks[128], char *tok) {
    int pos = -1;
    for (int i = 0; toks[i] != NULL; i++)
        if (strcmp(toks[i], tok) == 0) {
            pos = i;
            break;
        }
    return pos;
}

// Find an binary file path of a command
char * findCbin(char *paths[20], char *cmd) {
    char *cbin = mkstr(300);
    strcpy(cbin, "");
    for (int i = 0; paths[i] != NULL; i++) {
        strcpy(cbin, paths[i]);
        strcat(cbin, "/"); 
        strcat(cbin, cmd);
        
        if (access(cbin, X_OK) == 0) 
            break;
        else 
            strcpy(cbin, ""); 
    }
    return cbin;
}

char ** ltoks(char* line) {
    char **toks = strarr(128, 100); 
    getTokens(line, toks);
    return toks;
}

// Read a line from a file splitting into tokens
char ** lineToks(FILE* f) {
    size_t szl = 1000;
    char *line = mkstr(szl);
    if(getline(&line, &szl, f) == EOF)
        exit(0);
    
    // line = strtok(line, "\n");
    char **toks = strarr(128, 100); 
    // printf("getting tokens");
    getTokens(line, toks);
    return toks;
}

// Split a line into tokens
int getTokens(char* line, char *toks[128]) {
	char *dlm = " \n\t\r\f\v";
    toks[0] = strtok(line, dlm);
    // printf("got first token");
    if (toks[0] != NULL && strcmp(toks[0], "") == 0) 
        toks[0] = NULL;
    
    // printf("compared first token");
    int i = 0;
    if (toks[0] != NULL) {
        i = 1;
        while((toks[i] = strtok(NULL, dlm)) != NULL) {
            i++;
        }
    }
    // printf("finished getting tokens. returning...");
	return i;
}

// Create a string array
char ** strarr(int lna, int lns) {
    char **sarr = (char **) malloc(lna * sizeof(char *)); 
    for (int i = 0; i < lna; i++) 
        sarr[i] = mkstr(lns);
    
    return sarr;
}

// Allocate mem for a string
char * mkstr(int ln) {
    return (char *) malloc(ln * sizeof(char));
}

void cparr(char *dst[128], char *src[128]) {
    for (int i = 0; i < 128; i++) 
        strcpy(dst[i], src[i]);
}

