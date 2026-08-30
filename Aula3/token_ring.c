// Compile: gcc prodcons_1_thread_sem.c -o prodcons_1_thread_sem -pthread
// Execute: 

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define TOTAL_NODES 10
int ring[TOTAL_NODES];

int tkn = 0;

void *increment_token(void* ptr) {
  tkn++;
  int *int_node = (int*) ptr;
  printf("Node: %d - Token: %d\n", *int_node, tkn);
  printf("Exiting thread!\n");
  fflush(0);
  pthread_exit(0);
}

void create_node_threads(void) {
  pthread_t node_handle;
  
  for (int i = 0; i < TOTAL_NODES; i++) {
    ring[i] = i;
    if (pthread_create(&node_handle, 0, (void*) increment_token, (void*) &ring[i]) != 0) {
      printf("Error creating thread! Exiting\n");
      exit(0);
    }
  }

  pthread_join(node_handle, 0);

  getchar();
}

int main(void) {
  create_node_threads();
  printf("Terminate program!\n");
  fflush(0);
  exit(0);
}