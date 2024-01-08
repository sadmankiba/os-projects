#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> /* sysconf */
#include <stdio.h>

#include "mergesort.h"

#define MIN_THREAD_ELEM 16 /* Minimum number of elements to spawn a thread */

#define DEBUG 0
#define BRKDOWN_DEBUG 1

typedef struct {
    Elem *arr;
    int l;
    int r;
} SortArgs;

typedef struct {
    Elem *arr;
    int l1;
    int r1;
    int l2;
    int r2;
    Elem *B;
    int start;
} MergeArgs;

typedef struct {
    Elem *dest;
    Elem *src;
    int n;
} CopyArgs;

/* 
 * Returns -ve if a < b, 0 if a == b, +ve if a > b
 */
int compare_elem(const void *a, const void *b) {
    return *((int *) a) - *((int *) b);
}

void copy_elem(Elem *dest, Elem *src) {
    memcpy(dest->carr, src->carr, ELEM_MAX_LEN);
}

void * copy_arr_serial(void *args) {
    pthread_t thread_id = pthread_self();
    CopyArgs *copy_args = (CopyArgs *) args;
    Elem *dst = copy_args->dest;
    Elem *src = copy_args->src;
    int n = copy_args->n;
    if (DEBUG) {
        printf("%lu: In copy_arr_serial, n: %d\n", (unsigned long)thread_id, n);
        printf("%lu: Before serial copy, src: \n", (unsigned long)thread_id);
        for (int i = 0; i < n; i++) {
            printf("%lu: %d \n", (unsigned long) thread_id, src[i].i);
        }
        printf("%lu: \n", (unsigned long) thread_id);
    }
    for (int i = 0; i < n; i++)
        copy_elem(&dst[i], &src[i]);
    
    if (DEBUG) {
        printf("%lu: After serial copy, dst: \n", (unsigned long)thread_id);
        for (int i = 0; i < n; i++) {
            printf("%lu: %d \n", (unsigned long)thread_id, dst[i].i);
        }
        printf("%lu: \n", (unsigned long)thread_id);
    }

    return NULL;
}

void copy_arr_parallel(Elem *dst, Elem *src, int n) {
    if (DEBUG) {
        printf("Before parallel copy, src: \n");
        for (int i = 0; i < n; i++) {
            printf("%d ", src[i].i);
        }
        printf("\n");
    }

    int n_proc = sysconf(_SC_NPROCESSORS_ONLN);
    int chunk_size = n / n_proc;
    pthread_t threads[n_proc - 1];
    
    /* 
    We cannot create a single args within for loop and use it as argument for threads
    This is because the arg needs to remain valid during thread execution. 
    The thread only gets a pointer to the arg.
    */
    CopyArgs args[n_proc - 1];
    for (int i = 0; i < n_proc - 1; i++) {
        args[i].dest = dst + i * chunk_size;
        args[i].src = src + i * chunk_size;
        args[i].n = chunk_size;
    }
    for (int i = 0; i < n_proc - 1; i++) {
        pthread_create(&threads[i], NULL, copy_arr_serial, &args[i]);
    }
    CopyArgs args2 = {dst + (n_proc - 1) * chunk_size, src + (n_proc - 1) * chunk_size, n - chunk_size * (n_proc - 1)};
    
    copy_arr_serial(&args2);
    for (int i = 0; i < n_proc - 1; i++)
        pthread_join(threads[i], NULL);
    
    if (DEBUG) {
        printf("After parallel copy, dst: \n");
        for (int i = 0; i < n; i++) {
            printf("%d ", dst[i].i);
        }
        printf("\n");
    }
}

void insertion_sort(Elem *arr, int n) {
    Elem temp;
    for (int i = 1; i < n; i++) {
        copy_elem(&temp, &arr[i]);
        int j = i - 1;
        while (j >= 0 && compare_elem(&arr[j], &temp) > 0) {
            copy_elem(&arr[j + 1], &arr[j]);
            j--;
        }
        copy_elem(&arr[j + 1], &temp);
    }
}

/*
 * Merges two subarrays in same array
 * 
 * input
 * arr: array to sort
 * p: start index of left subarray
 * q: end index of left subarray. (q + 1) is start index of right subarray.
 * r: end index of right subarray
 */
void merge_serial(Elem *arr, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;

    Elem *L = (Elem *) malloc(n1 * sizeof(Elem));
    Elem *R = (Elem *) malloc(n2 * sizeof(Elem));

    /* L = A[p..q]. R = A[q+1..r] */
    for (int i = 0; i < n1; i++)
        copy_elem(&L[i], &arr[p + i]);
    
    for (int j = 0; j < n2; j++)
        copy_elem(&R[j], &arr[q + 1 + j]);

    int i = 0; /* number of elements merged from L */
    int j = 0; /* number of elements merged from R */
    int k = p; /* index on merged array */

    /* Copy smaller element between the two subarrays to merged array 
     until we reach end of one subarray */
    while (i < n1 && j < n2) {
        if(compare_elem(&L[i], &R[j]) <= 0) {
            copy_elem(&arr[k], &L[i]);
            i++;
        } else {
            copy_elem(&arr[k], &R[j]);
            j++;
        }
        k++;
    }

    /* Copy remaining */
    while (i < n1) {
        copy_elem(&arr[k], &L[i]);
        i++; k++;
    }

    while (j < n2) {
        copy_elem(&arr[k], &R[j]);
        j++; k++;
    }
}

/* 
 * Merges two subarrays in A to B
 */
void merge_serial_2(Elem *arr, int l1, int r1, int l2, int r2, Elem *B, int start) {
    int n1 = r1 - l1 + 1;
    int n2 = r2 - l2 + 1;

    int i = 0; /* number of elements merged from sarr1 */
    int j = 0; /* number of elements merged from sarr2 */
    int k = start;
    /* Copy smaller element between the two subarrays to merged array 
     until we reach end of one subarray */
    while (i < n1 && j < n2) {
        if(compare_elem(&arr[l1 + i], &arr[l2 + j]) <= 0) {
            copy_elem(&B[k], &arr[l1 + i]);
            i++;
        } else {
            copy_elem(&B[k], &arr[l2 + j]);
            j++;
        }
        k++;
    }

    /* Copy remaining */
    while (i < n1) {
        copy_elem(&B[k], &arr[l1 + i]);
        i++; k++;
    }

    while (j < n2) {
        copy_elem(&B[k], &arr[l2 + j]);
        j++; k++;
    }
}

/*
 * Find the element in arr that is just greater than or equal to p
 */
int find_split_point(Elem *arr, int l, int r, Elem *p) {
    int low = l;
    int high = r;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (compare_elem(p, &arr[mid]) <= 0)
            high = mid;
        else
            low = mid + 1;
    }
    return low;
}

void * merge_parallel_aux(void *args) {
    MergeArgs *merge_args = (MergeArgs *) args;
    Elem *arr = merge_args->arr;
    int l1 = merge_args->l1;
    int r1 = merge_args->r1;
    int l2 = merge_args->l2;
    int r2 = merge_args->r2;
    Elem *B = merge_args->B;
    int start = merge_args->start;

    if ((r1 - l1 + 1) <= MIN_THREAD_ELEM) {
        merge_serial_2(arr, l1, r1, l2, r2, B, start);
        return NULL;
    }

    if (l1 > r1 && l2 > r2)
        return NULL;
    
    if ((r2 - l2) < (r1 - l1)) {
        int temp;
        temp = l1; l1 = l2; l2 = temp;
        temp = r1; r1 = r2; r2 = temp;
    }
    
    int pi = l2 + (r2 - l2) / 2;
    Elem pivot;
    copy_elem(&pivot, &arr[pi]);
    int s = find_split_point(arr, l1, r1, &pivot);

    copy_elem(&B[start + (s - l1) + (pi - l2)], &pivot);
    
    /* Let main thread merge one subarray, a spawned thread merge another */
    pthread_t t;
    MergeArgs args1 = {arr, l1, s - 1, l2, pi - 1, B, start};
    MergeArgs args2 = {arr, s, r1, pi + 1, r2, B, start + (s - l1) + (pi - l2) + 1};
    pthread_create(&t, NULL, merge_parallel_aux, &args2);
    
    merge_parallel_aux(&args1);
    
    pthread_join(t, NULL);
    return NULL;
}

void merge_parallel(Elem *arr, int l, int m, int r) {
    Elem *B = (Elem *) malloc((r - l + 1) * sizeof(Elem));
    MergeArgs args = {arr, l, m, m + 1, r, B, 0};
    merge_parallel_aux(&args);

    if (DEBUG) {
        printf("After parallel merge, B: \n");
        for (int i = 0; i < r - l + 1; i++) {
            printf("%d ", B[i].i);
        }
        printf("\n");
    }

    copy_arr_parallel(arr + l, B, r - l + 1);

    if (DEBUG) {
        printf("After parallel copy, arr: \n");
        for (int i = l; i <= r; i++) {
            printf("%d ", arr[i].i);
        }
        printf("\n");
    }
}

void * merge_sort_threaded(void *args) {
    SortArgs *sort_args = (SortArgs *) args;
    Elem *arr = sort_args->arr;
    int l = sort_args->l;
    int r = sort_args->r;

    if ((r - l + 1) <= MIN_THREAD_ELEM) {
        insertion_sort(arr + l, r - l + 1);
        return NULL;
    }

    if (l < r) {
        int m = l + (r - l) / 2;
        pthread_t self = pthread_self();

        pthread_t t;
        SortArgs args1 = {arr, l, m};
        SortArgs args2 = {arr, m + 1, r};
        /* Let main thread sort one subarray, a spawned thread sort another */
        clock_t start, end;
        start = clock();
        pthread_create(&t, NULL, merge_sort_threaded, &args2);
        merge_sort_threaded(&args1);
        
        pthread_join(t, NULL);
        end = clock();
        if (BRKDOWN_DEBUG) {
            printf("%lu: Time taken for subarray sort (%d): %f\n", (unsigned long) self, r - l + 1, (double) (end - start) / CLOCKS_PER_SEC);
        }
        if (DEBUG) {
            printf("After subarrays sorted: \n");
            for (int i = l; i <= r; i++) {
                printf("%d ", arr[i].i);
            }
            printf("\n");
        }
        start = clock();
        merge_parallel(arr, l, m, r);
        end = clock();
        if (BRKDOWN_DEBUG) {
            printf("%lu: Time taken for merge (%d): %f\n", (unsigned long) self, r - l + 1, (double) (end - start) / CLOCKS_PER_SEC);
        }
    }

    return NULL;
}

void merge_sort_parallel(Elem *arr, int l, int r) {
    SortArgs args = {arr, l, r};
    merge_sort_threaded(&args);
}

void merge_sort_serial(Elem *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        merge_sort_serial(arr, l, m);
        merge_sort_serial(arr, m + 1, r);
        merge_serial(arr, l, m, r);
    }
}

void bubble_sort_elem(Elem *arr, int n) {
    Elem temp;
    for(int j = 0; j < (n - 1); j++) {
        for (int i = 0; i < (n - j - 1); i++) {
            if (compare_elem(&arr[i], &arr[i + 1]) > 0) {
                copy_elem(&temp, &arr[i]);
                copy_elem(&arr[i], &arr[i + 1]);
                copy_elem(&arr[i + 1], &temp);
            }
        }
    }       
}


