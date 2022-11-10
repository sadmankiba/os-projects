#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
    // FILE *fr = fopen("input", "rb");
    // FILE *fw = fopen("output_my", "wb");
    
    char ERRMSG[30] = "An error has occurred\n";
    int wr;
    if (argc != 3 || access(argv[1], R_OK) != 0) {
        wr = write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));	
        exit(0);
    }
    FILE *fr = fopen(argv[1], "rb");
    FILE *fw = fopen(argv[2], "wb");
    int arr[10000 * 25];
    
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

	// printf("%d ", arr[(n - 1) * 25]);
    }
    // printf("\n");
    // printf("n = %d\n", n);
    
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
    
    for (int i = 0; i < n; i++) {
        // printf("%d ", arr[i * 25]);
    }

    wr = fwrite(arr, sizeof(int), n * 25, fw);
    if (wr != n * 25) {
        exit(0);
    }	
    fclose(fr);
    fclose(fw);

    return 0;
}
