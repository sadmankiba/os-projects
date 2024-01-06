#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "mergesort.h"

char ERRMSG[30] = "An error has occurred\n";

void bubbleSort(int *arr, int n);

int main (int argc, char *argv[]) {
    int wr;
    if (argc != 3 || access(argv[1], R_OK) != 0) {
        wr = write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));	
        exit(0);
    }
    FILE *fr = fopen(argv[1], "rb");
    FILE *fw = fopen(argv[2], "wb");
    int *arr = (int *) malloc(1000000 * 25 * sizeof(int)); // 100MB
    
    int n = 0;
    while (true) {
        if (fread(&(arr[n * 25]), sizeof(int), 25, fr) == 25) {
	        n++;
	    }
        else {
            if (n == 0){
                wr = write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
                exit(0);
            }
            break;
	    }
        if (n == 1000000)
            break;
    }
    
    parallel_merge_sort(arr, 0, n - 1);
    // bubbleSort(arr, n);
    
    for (int i = 0; i < n; i++) {
        // printf("%d ", arr[i * 25]);
    }

    wr = fwrite(arr, sizeof(int), n * 25, fw);
    if (wr != n * 25) {
        exit(0);
    }	
    // printf("Elapsed time: %lu secs.\n", clock() / CLOCKS_PER_SEC);

    fclose(fr);
    fclose(fw);

    return 0;
}

void bubbleSort(int *arr, int n) {
    int temp[25];
    for(int j = 0; j < (n - 1); j++)
        for (int i = 0; i < (n - 1); i++)
            if (arr[i * 25] > arr[(i + 1) * 25]) {
                for (int k = 0; k < 25; k++) {
                    temp[k] = arr[i * 25 + k];
                    arr[i * 25 + k] = arr[(i + 1) * 25 + k];
                    arr[(i + 1) * 25 + k] = temp[k];
                }
            }
}

