// to compile: gcc tokenring_threads_sem.c -o tokenring_threads_sem -lpthread
// to execute: tokenring_threads_sem
//
// This code simulates a token ring, using THREADS + 1 lightweight processes (or threads), being 01 master and more THREAD threads.
// The master code creates THREADS threads and releases the first thread, using a semaphore.
// Each thread unlocks, increases tkn and then releases the next thread, in a circular queue.
// Master thread, when released, prints the new tkn.
//
// Code uses POSIX Semaphores (IEEE Std 1003.1b).
//
// copyrights: Paulo S. L. de Souza and Davi J. Conte
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define THREADS 7

sem_t  permit[THREADS];
int tkn = 0;

void *children(void *p_arg) {
	int p, next;

	p = (int) *((int *)p_arg);
	next = (p + 1) % THREADS;

	printf("Node %d, tkn %d, next %d \n", p, tkn, next);

	sem_wait(&permit[p]);	
	tkn ++;	
	printf("Node %d with token %d \n", p,tkn);
	//if (next < THREADS)
	  sem_post(&permit[next]);

	pthread_exit(0);
} // end children


int main(void) {
	pthread_t handle[THREADS];

	int i, p[THREADS], initial = 0;

	p[0] = 0;
	sem_init (&permit[0], 0 , initial);

	for (i=1; i < THREADS; i++) {
		p[i] = i;
		sem_init (&permit[i], 0 , initial);
		if (pthread_create(&handle[ i ], 0, (void *) children, (void *)&p[i] /*&i*/) != 0) { 
			printf("Error creating thread %d! Exiting! \n", i);
			fflush(0);
			exit(0);
		}
	}
	// thread main releases first thread and waits from last one, after pthread_join()
	sem_post(&permit[1]);

	for (i=1; i < THREADS; i++) {
		pthread_join(handle[ i ], 0);
		sem_destroy(&permit[i]);
		printf("Node %d out! \n", i);
	}
	printf("Token = %d \n", ++tkn);
	exit(0);
} // end main()