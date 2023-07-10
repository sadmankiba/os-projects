#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

/* Compile with -lpthread */

/*
 * Reader-writer problem
 * Without the lock, sometimes "Write: N" is printed after "Read: N"  
 */

#define MAX_READERS 10

int resrc = 0;
sem_t rd;
sem_t wrt;

int n_rdrs = 0;

void * read_resrc(void *args) {
    sem_wait(&rd);
    n_rdrs++;
    if (n_rdrs == 1) 
        sem_wait(&wrt);
    sem_post(&rd);
    for (int i = 0; i < 10; i++)
        printf("Read: %d\n", resrc);
    
    sem_wait(&rd);
    n_rdrs--;
    if (n_rdrs == 0) 
        sem_post(&wrt);
    sem_post(&rd);
}

void * write_resrc(void *args) {
    for (int i = 0; i < 10; i++) {
        sem_wait(&wrt);
        resrc = i + 1;
        printf("Write: %d\n", resrc);
        sem_post(&wrt);
    }
}

int main() {
    sem_init(&rd, 0, 1);
    sem_init(&wrt, 0, 1);

    pthread_t read_thrd[MAX_READERS], writ_thrd;
    for (int i = 0; i < MAX_READERS; i++) {
        pthread_create(&(read_thrd[i]), NULL, read_resrc, NULL);
    }
    pthread_create(&writ_thrd, NULL, write_resrc, NULL);

    for (int i = 0; i < MAX_READERS; i++) {
        pthread_join(read_thrd[i], NULL);
    }
    pthread_join(writ_thrd, NULL);
}