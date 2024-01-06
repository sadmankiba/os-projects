
void parallel_merge_25(int *arr, int p, int q, int r) {
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

/*
 * Merge stage in mergesort of an array
 * 
 * input
 * arr: array to sort
 * p: start index of left subarray
 * q: end index of left subarray. (q + 1) is start index of right subarray.
 * r: end index of right subarray
 */
void serial_merge(int *arr, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;

    int L[n1], M[n2];

    /* L = A[p..q]. M = A[q+1..r] */
    for (int i = 0; i < n1; i++)
        L[i] = arr[p + i];
    for (int j = 0; j < n2; j++)
        M[j] = arr[q + 1 + j];

    int i = 0; /* number of elements merged from L */
    int j = 0; /* number of elements merged from M */
    int k = p; /* index on merged array */

    /* Copy smaller element between the two subarrays to merged array 
     until we reach end of one subarray */
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

    /* Copy remaining */
    while (i < n1) {
        arr[k] = L[i]; i++; k++;
    }

    while (j < n2) {
        arr[k] = M[j]; j++; k++;
    }
}

void parallel_merge_sort(int *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        parallel_merge_sort(arr, l, m);
        parallel_merge_sort(arr, m + 1, r);
        parallel_merge_25(arr, l, m, r);
    }
}

void serial_merge_sort(int *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;

        serial_merge_sort(arr, l, m);
        serial_merge_sort(arr, m + 1, r);
        serial_merge(arr, l, m, r);
    }
}


