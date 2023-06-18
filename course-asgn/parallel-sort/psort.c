#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>

void mergeSort(int *arr, int l, int r);
void bubbleSort(int *arr, int n);
void merge25(int *arr, int p, int q, int r); 
void merge(int *arr, int p, int q, int r);

int main (int argc, char *argv[]) {
    // FILE *fr = fopen("hndrdk-in", "rb");
    // FILE *fw = fopen("hndrdk-my-out", "wb");
    
    char ERRMSG[30] = "An error has occurred\n";
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
    
    mergeSort(arr, 0, n - 1);
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

void mergeSort(int *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge25(arr, l, m, r);
    }
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

void merge25(int *arr, int p, int q, int r) {
    // L = A[p..q]. M = A[q+1..r]
    int n1 = q - p + 1;
    int n2 = r - q;

    int *L = (int *) malloc(n1 * 25 * sizeof(int));
    int *M = (int *) malloc(n2 * 25 * sizeof(int));

    for (int i = 0; i < n1 * 25; i++)
        L[i] = arr[p * 25 + i];
    for (int j = 0; j < n2 * 25; j++)
        M[j] = arr[(q + 1) * 25 + j];

    int i, j, k;
    i = 0; j = 0; k = p;

    // Copy larger until we reach end of L or M
    while (i < n1 && j < n2) {
        if (L[i * 25] <= M[j * 25]) {
            for (int h = 0; h < 25; h++)
                arr[k * 25 + h] = L[i * 25 + h];
            i++;
        } else {
            for (int h = 0; h < 25; h++)
                arr[k * 25 + h] = M[j * 25 + h];
            j++;
        }
        k++;
    }

    // Copy remaining
    while (i < n1) {
        for (int h = 0; h < 25; h++)
            arr[k * 25 + h] = L[i * 25 + h];
             
        i++; k++;
    }

    while (j < n2) {
        for (int h = 0; h < 25; h++)
            arr[k * 25 + h] = M[j * 25 + h];
            
        j++; k++;
    }
    assert(k == r + 1);
}

void merge(int *arr, int p, int q, int r) {
    // L = A[p..q]. M = A[q+1..r]
    int n1 = q - p + 1;
    int n2 = r - q;

    int L[n1], M[n2];

    for (int i = 0; i < n1; i++)
        L[i] = arr[p + i];
    for (int j = 0; j < n2; j++)
        M[j] = arr[q + 1 + j];

    int i, j, k;
    i = 0; j = 0; k = p;

    // Copy larger until we reach end of L or M
    while (i < n1 && j < n2) {
        if (L[i] <= M[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = M[j];
            j++;
        }
        k++;
    }

    // Copy remaining
    while (i < n1) {
        arr[k] = L[i]; i++; k++;
    }

    while (j < n2) {
        arr[k] = M[j]; j++; k++;
    }
}

