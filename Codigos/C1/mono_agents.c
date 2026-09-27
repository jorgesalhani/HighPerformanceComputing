#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_PROD 30
#define MAX_QUEUE 7

sem_t mutex, empty, full;

int queue[MAX_QUEUE], item_available, produced=0, consumed=0;

int create_item(void) {
    return rand()%1000;
}

void insert_into_queue(int item) {
    queue[item_available++] = item;
    produced++;
    printf("[Producing] ID item: %d, Item: %d, Queue: %d\n", produced, item, item_available);
    return;
}

int extract_from_queue() {
    consumed++;
    printf("[Consuming] ID item: %d, Item: %d, Queue: %d\n", consumed, queue[item_available-1], item_available-1);
    return queue[--item_available];
}

void process_item(int item) {
    static int printed=0;
    printf("[Processing] ID item: %d, Item: %d, Queue: %d\n", printed, item, item_available);
    return;
}

void *producer(void) {
    int item, local_produced;

    do {
        item = create_item();

        sem_wait(&empty);
        sem_wait(&mutex);

        local_produced = produced;
        if (local_produced < MAX_PROD) {
            insert_into_queue(item);
            local_produced = produced;
            sem_post(&mutex);
            sem_post(&full);
        } else {
            sem_post(&mutex);
            sem_post(&empty);
        }
    } while (local_produced < MAX_PROD);

    printf("Finish producer!\n");
    fflush(0);
    pthread_exit(0);
}

void *consumer(void) {
    int item = 0, local_consumed;

    do {
        sem_wait(&full);
        sem_wait(&mutex);

        local_consumed = consumed;
        if (local_consumed < MAX_PROD) {
            item = extract_from_queue();
            local_consumed = consumed;
            sem_post(&mutex);
            sem_post(&empty);

            process_item(item);
        } else {
            sem_post(&mutex);
            sem_post(&full);
        }
    } while (local_consumed < MAX_PROD);

    printf("Finish consumer!\n");
    fflush(0);
    pthread_exit(0);
}

int main(void) {
    pthread_t prod_handle, cons_handle;

    item_available = 0;

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, MAX_QUEUE);
    sem_init(&full, 0, 0);

    if (pthread_create(&prod_handle, 0, (void*) producer, (void*) 0) != 0) {
        printf("Error creating thread!\n");
        exit(0);
    }

    if (pthread_create(&cons_handle, 0, (void*) consumer, (void*) 0) != 0) {
        printf("Error creating thread!\n");
        exit(0);
    }

    pthread_join(prod_handle, 0);
    pthread_join(cons_handle, 0);

    // getchar();
    fflush(0);

    exit(0);
}