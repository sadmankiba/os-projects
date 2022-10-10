#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
    int batch = 0;
    FILE *fptr;
    if(argc > 1) {
        batch = 1;
        fptr = fopen(argv[1], "r");
    }
    while(1) {
        if (batch == 0)
            printf("wish> ");
        size_t bln = 100;
        char *bin = (char *) malloc(bln * sizeof(char));
        if(batch == 1) {
            fgets(bin, bln, fptr);
        } else {
            getline(&bin, &bln, stdin);
        }
        bin[strlen(bin)-1]='\0';
        if(strcmp(bin, "exit") == 0) {
            exit(0);
        } else {
            printf("%s", bin);
        }
    }
}