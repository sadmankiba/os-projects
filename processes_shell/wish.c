#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main () {
    while(1) {
        printf("wish> ");
        size_t bln = 100;
        char *bin = (char *) malloc(bln * sizeof(char));

        int chrs = getline(&bin, &bln, stdin);
        bin[chrs-1]='\0';
        if(strcmp(bin, "exit") == 0) {
            exit(0);
        } else {
            printf("%s", bin);
        }
    }
}