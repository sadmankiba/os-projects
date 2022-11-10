#include <stdio.h>

int main () {
    FILE *fr = fopen("input.txt", "r");
    FILE *fw = fopen("output.txt", "w");
    int arr[10];
    for (int i = 0; i < 10; i++)
        fscanf(fr, "%d", &arr[i]);
    
    for(int j = 0; j < 9; j++)
        for (int i = 0; i < 9; i++)
            if (arr[i] > arr[i+1]) {
                int temp = arr[i];
                arr[i] = arr[i+1];
                arr[i+1] = temp;
            }
    
    for (int i = 0; i < 10; i++)
        fprintf(fw, "%d ", arr[i]);
    
    fclose(fr);
    fclose(fw);

    return 0;
}