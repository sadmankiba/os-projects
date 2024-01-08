#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mergesort.h"

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "%s:%d: Assertion `%s` failed: %s\n", __FILE__, __LINE__, #condition, (message)); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)


void test_bubble_sort_elem() {
    Elem arr[10];
    for (int i = 0; i < 10; i++)
        arr[i].i = 10 - i;
    
    bubble_sort_elem(arr, 10);

    for (int i = 0; i < 10; i++)
        ASSERT(arr[i].i == i + 1, "Bubble sort failed");
    
    for (int i = 0; i < 10; i++) {
        int temp = 10 - i;
        strncpy(arr[i].carr, (char *) &temp, 4);
    }
    
    bubble_sort_elem(arr, 10);

    for (int i = 0; i < 10; i++) {
        ASSERT(*((int *) &arr[i]) == i + 1, "Bubble sort failed");
    }

    printf("Bubble sort element passed\n");
}

void test_merge_sort_serial() {
    Elem arr[10];
    for (int i = 0; i < 10; i++) {
        int temp = 10 - i;
        strncpy(arr[i].carr, (char *) &temp, 4);
    }

    merge_sort_serial(arr, 0, 10 - 1);
    
     for (int i = 0; i < 10; i++) {
        ASSERT(*((int *) &arr[i]) == i + 1, "Merge sort serial failed");
    }
    
    printf("Merge sort serial passed\n");
}

void test_copy_arr_parallel() {
    Elem arr[10];
    for (int i = 0; i < 10; i++) {
        int temp = 10 - i;
        strncpy(arr[i].carr, (char *) &temp, 4);
    }

    Elem arr2[10];
    copy_arr_parallel(arr2, arr, 10);
    
     for (int i = 0; i < 10; i++) {
        ASSERT(*((int *) &arr2[i]) == 10 - i, "Copy arr parallel failed");
    }
    
    printf("Copy arr parallel passed\n");
}

void test_merge_sort_parallel() {
    Elem arr[10];
    for (int i = 0; i < 10; i++) {
        int temp = 10 - i;
        strncpy(arr[i].carr, (char *) &temp, 4);
    }

    merge_sort_parallel(arr, 0, 10 - 1);
    
     for (int i = 0; i < 10; i++) {
        ASSERT(*((int *) &arr[i]) == i + 1, "Merge sort parallel 10 failed");
    }

    Elem arr2[20];
    for (int i = 0; i < 20; i++) {
        int temp = 20 - i;
        strncpy(arr2[i].carr, (char *) &temp, 4);
    }
    merge_sort_parallel(arr2, 0, 20 - 1);
    
     for (int i = 0; i < 20; i++) {
        ASSERT(*((int *) &arr2[i]) == i + 1, "Merge sort parallel 20 failed");
    }
    
    printf("Merge sort parallel passed\n");
}

int main() {
    test_bubble_sort_elem();
    test_merge_sort_serial();
    test_copy_arr_parallel();
    test_merge_sort_parallel();
    return 0;
}