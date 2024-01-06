#include <stdio.h>

#include "mergesort.h"

void test_serial() {
    int arr[10];
    for (int i = 0; i < 10; i++)
        arr[i] = 10 - i;

    serial_merge_sort(arr, 0, 10 - 1);
    
    for (int i = 0; i < 10; i++)
        assert(arr[i] == i + 1, "Serial merge sort failed");
    
    printf("Serial merge sort passed\n");
}