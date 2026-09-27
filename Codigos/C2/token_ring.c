#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define THREADS 7

sem_t permit[THREADS];
int token = 0;

void *children(void *p_arg) {
    int p, next;

    p = (int) *((int*) p_arg);
    next = (p+1) % THREADS;

    printf("Node: %d, Token: %d, Next: %d\n", p, token, next);

    sem_wait(&permit[p]);
    token++;
    printf("Node: %d, Token: %d\n", p, token);
    sem_post(&permit[next]);

    pthread_exit(0);
}

int main(void) {
    pthread_t handle[THREADS];

    int p[THREADS], initial = 0;
    p[0] = 0;

    sem_init(&permit[0], 0, initial);

    for (int i = 1; i < THREADS; i++) {
        p[i] = i;
        sem_init(&permit[i], 0, initial);
        if (pthread_create(&handle[i], 0, (void*) children, (void*) &p[i]) != 0) {
            printf("Error creating thread\n");
            fflush(stdout);
            exit(0);
        }
    }

    sem_post(&permit[1]);

    for (int i = 1; i < THREADS; i++) {
        pthread_join(handle[i], 0);
        sem_destroy(&permit[i]);
        printf("Node: %d out\n", i);
    }

    printf("Token: %d\n", ++token);
    return 0;
}