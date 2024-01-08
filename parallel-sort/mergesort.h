#ifndef MERGESORT_H
#define MERGESORT_H

/*
 * Represents an array element to be sorted.
 * The element can be either an integer or 
 * a key-value with first 4 bytes as key and 
 * the rest as value.
 */

#define ELEM_MAX_LEN 100

typedef union {
    int i;
    char carr[ELEM_MAX_LEN];
} Elem;

void copy_arr_parallel(Elem *dst, Elem *src, int n);

void merge_sort_parallel(Elem *arr, int l, int r);
void merge_sort_serial(Elem *arr, int l, int r);
void bubble_sort_elem(Elem *arr, int n);

#endif 