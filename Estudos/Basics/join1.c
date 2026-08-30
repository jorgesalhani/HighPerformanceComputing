#include <stdio.h>
#include <pthread.h>

#define NTHREADS 10

void *thread_function(void*);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

int main(void) {
  pthread_t thread_id[NTHREADS];
  int i,j;

  for (i = 0; i < NTHREADS; i++) {
    pthread_create(&thread_id[i], NULL, thread_function, NULL);
  }

  for (j = 0; j < NTHREADS; j++) {
    pthread_join(thread_id[j], NULL);
  }

  printf("Final counter value: %d\n", counter);
}

void *thread_function(void *dummyPtr) {
  printf("Thread number %ld: Update counter %d\n", pthread_self(), counter);
  pthread_mutex_lock(&mutex1);
  counter++;
  pthread_mutex_unlock(&mutex1);
}