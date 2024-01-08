#include <stdio.h>
#include <fcntl.h>  /* open */
#include <unistd.h> /* close */
#include <sys/mman.h> /* mmap */
#include <time.h> /* clock */
#include <stdlib.h> /* atoi */
#include <string.h> /* strlen */


#include "mergesort.h"

char ERRMSG[30] = "An error has occurred\n";

enum SortAlgo {
    BUBBLE_SORT = 1,
    MERGE_SORT_SERIAL,
    MERGE_SORT_PARALLEL
};

/*
 * Takes 3 command line arguments:
 * 1. Input file name
 * 2. Output file name
 * 3. Sort algorithm to use
 */
int main (int argc, char *argv[]) {
    if (argc != 4 || access(argv[1], R_OK) != 0) {
        write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));	
        exit(0);
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
        exit(0);
    }
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out == -1) {
        write(STDERR_FILENO, ERRMSG, strlen(ERRMSG));
        exit(0);
    }

    unsigned long long file_size = lseek(fd, 0, SEEK_END);
    unsigned int n = file_size / ELEM_MAX_LEN;
    lseek(fd, 0, SEEK_SET); // Reset the file offset to the beginning
    
    printf("File size: %llu, num elements: %u\n", file_size, n);

    void *buf = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    
    clock_t start, end;

    switch (atoi(argv[3])) {
        case BUBBLE_SORT:
            printf("Running bubble sort\n");
            start = clock();
            bubble_sort_elem(buf, n);
            end = clock();
            break;
        case MERGE_SORT_SERIAL:
            printf("Running merge sort serial\n");
            start = clock();
            merge_sort_serial(buf, 0, n - 1);
            end = clock();
            break;
        case MERGE_SORT_PARALLEL:
            printf("Runnin merge sort parallel\n");
            start = clock();
            merge_sort_parallel(buf, 0, n - 1);
            end = clock();
            break;
    }

    write(fd_out, buf, file_size);
    printf("Sort time: %f secs.\n", ((double) end - start) / CLOCKS_PER_SEC);

    close(fd);
    close(fd_out);
    munmap(buf, file_size);    
    return 0;
}

