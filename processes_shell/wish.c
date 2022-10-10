#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int findRdir(char *toks);
char * findCbin(char *paths[20], char *cmd);
char ** lineToks(FILE* f);
int getTokens(char* line, char *toks[128]);
char ** strarr(int lna, int lns);
char * mkstr(int ln);

char *EXITCMD = "exit";
char *CDCMD = "cd";
char *PATHCMD = "path";

int main (int argc, char* argv[]) {
    int batch = argc > 1? 1: 0;
    FILE *f = batch == 1? fopen(argv[1], "r"): stdin;
    char *paths[20] = strarr(20, 50);
    strcpy(paths[0], "/bin");
    paths[1] = NULL;

    while(1) {
        if (batch == 0)
            printf("wish> ");
        
        char *toks[128] = lineToks(f);
        int rdpos = findRdir(toks);
        if (rdpos >=1)
            toks[rdpos] = NULL;
        
        if(strcmp(toks[0], EXITCMD) == 0) {
            exit(0);
        } else if(strcmp(toks[0], CDCMD) == 0) {
            chdir(toks[1]);
        } else if(strcmp(toks[0], PATHCMD) == 0) {
            for (int i = 0; toks[i] != NULL; i++)
                paths[i] = toks[i+1];
        } else {
            char *cbin = findCbin(paths, toks[0]);

            if (strcmp(cbin, "") != 0) {
                if(rdpos >= 1) {
                    int fd = open(toks[rdpos+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
                pid_t pid = fork();
                if (pid < 0) {
                    printf("fork error");
                    exit(1);
                } else if (pid == 0) {
                    execv(cbin, toks);
                } else {
                    wait(NULL);
                }
            }
            else
                printf("error: cbin not found");
        }
    }
}

int findRdir(char *toks) {
    int pos = -1;
    for (int i = 0; toks[i] != NULL; i++)
        if (strcmp(toks[i], ">")) {
            pos = i;
            break;
        }
    return pos;
}

char * findCbin(char *paths[20], char *cmd) {
    char *cbin = mkstr(50);
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

char ** lineToks(FILE* f) {
    size_t szl = 1000;
    char *line = mkstr(szl);
    if(getline(&line, &szl, f) == EOF)
        exit(0);
    
    // line = strtok(line, "\n");
    char *toks[128] = strarr(128, 100); 
    getTokens(line, toks);
    return toks;
}

int getTokens(char* line, char *toks[128]) {
	char *dlm = " \n\t\r\f\v";
    toks[0] = strtok(line, dlm);
    int i = 1;
    while((toks[i] != strtok(NULL, dlm)) != NULL) {
        i++;
    }
	return i;
}

char ** strarr(int lna, int lns) {
    char *sarr[lna]; 
    for (int i = 0; i < lna; i++) 
        sarr[i] = mkstr(lns);
    
    return sarr;
}

char * mkstr(int ln) {
    return (char *) malloc(ln * sizeof(char));
}

void cparr(char *dst[128], char *src[128]) {
    for (int i = 0; i < 128; i++) 
        strcpy(dst[i], src[i]);
}

