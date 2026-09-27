#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_PROD 30
#define MAX_QUEUE 7
#define NUM_PROD 4
#define NUM_CONS 4

sem_t mutex, empty, full;

int queue[MAX_QUEUE];
int in = 0, out = 0, count = 0;
int produced = 0, consumed = 0;

int create_item(int thread_id) {
    return rand() % 1000 + (thread_id * 1000);
}

void process_item(int item, int item_id, int thread_id) {
    printf("[Processing - Consumer %d] ID item: %d, Value: %d\n", thread_id, item_id, item);
}

void insert_into_queue(int item, int thread_id) {
    queue[in] = item;
    in = (in + 1) % MAX_QUEUE;
    count++;
    produced++;
    printf("[Producer %d] ID Item: %d, Value: %d (Buffer : %d)\n", thread_id, produced, item, count);
}

int extract_from_queue(int thread_id, int* item_val) {
    *item_val = queue[out];
    out = (out+1) % MAX_QUEUE;
    count--;
    consumed++;
    printf("[Consumer %d] ID Item: %d, Value: %d (Buffer : %d)\n", thread_id, consumed, *item_val, count);
    return consumed;
}

void *producer(void *args) {
    int tid = *(int*) args;
    int item; 
    int local_produced;

    do {
        item = create_item(tid);
        sem_wait(&empty);
        sem_wait(&mutex);

        local_produced = produced;
        if (local_produced < MAX_PROD) {
            insert_into_queue(item, tid);
            local_produced = produced;
            sem_post(&mutex);
            sem_post(&full);
        } else {
            sem_post(&mutex);
            sem_post(&empty);
            sem_post(&full);
        }
    } while (local_produced < MAX_PROD);

    printf("Producer finished: %d\n", tid);
    fflush(stdout);
    pthread_exit(NULL);
}

void *consumer(void *args) {
    int tid = *(int*) args;
    int item = 0;
    int item_id = 0;
    int local_consumed;

    do {
        sem_wait(&full);
        sem_wait(&mutex);

        local_consumed = consumed;
        if (local_consumed < MAX_PROD) {
            item_id = extract_from_queue(tid, &item);
            local_consumed = consumed;
            sem_post(&mutex);
            sem_post(&empty);

            process_item(item, item_id, tid);
        } else {
            sem_post(&mutex);
            sem_post(&full);
            sem_post(&empty);
        }
    } while (local_consumed < MAX_PROD);

    printf("Consumer finished: %d\n", tid);
    fflush(stdout);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t prod_threads[NUM_PROD];
    pthread_t cons_threads[NUM_CONS];
    int prod_ids[NUM_PROD];
    int cons_ids[NUM_CONS];

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, MAX_QUEUE);
    sem_init(&full, 0, 0);

    for (int i = 0; i < NUM_PROD; i++) {
        prod_ids[i] = i+1;
        if (pthread_create(&prod_threads[i], NULL, producer, &prod_ids[i]) != 0) {
            perror("Error creating producert thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_CONS; i++) {
        cons_ids[i] = i+1;
        if (pthread_create(&cons_threads[i], NULL, consumer, &cons_ids[i]) != 0) {
            perror("Error creating consumer thread\n");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_PROD; i++) {
        pthread_join(prod_threads[i], NULL);
    }

    for (int i = 0; i < NUM_CONS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    printf("Finishing!\n");
    fflush(stdout);
    return 0;
}