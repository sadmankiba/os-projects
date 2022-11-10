#include <stdio.h>

int main (int argc, char *argv[]) {
    FILE *fr = fopen(argv[1], "rb");
    FILE *fw = fopen(argv[2], "wb");
    int arr[10 * 25];

    fread(arr, sizeof(int), 10 * 25, fr);

    for (int i = 0; i < 10; i++) {
        // fscanf(fr, "%d", &arr[i]);
        printf("%d ", arr[i * 25]);
        // int temp;
        // for (int j = 0; j < 24; j++) {
        //     fscanf(fr, "%d", &temp);
        // }
    }
    printf("\n");
    
    for(int j = 0; j < 9; j++)
        for (int i = 0; i < 9; i++)
            if (arr[i * 25] > arr[(i + 1) * 25]) {
                int temp[25];
                for (int k = 0; k < 25; k++) {
                    temp[k] = arr[i * 25 + k];
                    arr[i * 25 + k] = arr[(i + 1) * 25 + k];
                    arr[(i + 1) * 25 + k] = temp[k];
                }
            }
    
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i * 25]);
    }

    fwrite(arr, sizeof(int), 10 * 25, fw);
    
    fclose(fr);
    fclose(fw);

    return 0;
}