#include <stdio.h>
#include <stdbool.h>

int main (int argc, char *argv[]) {
    // FILE *fr = fopen("input", "rb");
    // FILE *fw = fopen("output_my", "wb");

    FILE *fr = fopen(argv[1], "rb");
    FILE *fw = fopen(argv[2], "wb");
    int arr[1000 * 25];
    
    int n = 0;
    while (true) {
        if (fread(&(arr[n * 25]), sizeof(int), 25, fr) == 25) {
	    n++;
	}
        else break;

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

    fwrite(arr, sizeof(int), n * 25, fw);
    
    fclose(fr);
    fclose(fw);

    return 0;
}
