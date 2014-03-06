#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

/* Arbitrary numbers for thread_count and num_loops */
#define THREAD_COUNT 5
#define NUM_LOOPS 3

/* Global variables */
const int thread_count = THREAD_COUNT;
int item;
sem_t sem_array[THREAD_COUNT];
pthread_mutex_t mutex;
int consumed = 0;
unsigned int seed;

/* Thread function */
void* thread_work(void* rank);

/* Main */
int main(int argc, char *argv[])
{
	long thread;
	pthread_t* thread_handles;
	
	seed = time(NULL);
	
	/* Allocate space for thread_handles */
	thread_handles = malloc(thread_count*sizeof(pthread_t));
	
	/* Initialize semaphores and mutex */
	sem_init(&sem_array[0], 0, 1);
	for (thread=1; thread<thread_count; thread++) {
		sem_init(&sem_array[thread], 0, 0);
	}
	pthread_mutex_init(&mutex, NULL);
	
	/* Start threads */
	for (thread=0; thread<thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, thread_work, (void*) thread);
	}
	
	/* Join threads */
	for (thread=0; thread<thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	
	/* Destroy semaphores and mutex */
	for (thread=0; thread<thread_count; thread++) {
		sem_destroy(&sem_array[thread]);
	}
	pthread_mutex_destroy(&mutex);
	
	/* Free allocated space for thread_handles and exit */
	free(thread_handles);
	return 0;
}



void* thread_work(void* rank)
{
	long my_rank = (long) rank;
	int i, j;
	
	/* Run the loop an arbitrary number of times */
	for (i=0; i<NUM_LOOPS; i++) {
		if (my_rank == 0) {
			/* Producer - wait on producer semaphore,
			place random number in buffer, signal consumer semaphores */
			sem_wait(&sem_array[my_rank]);
			item = rand_r(&seed)%100;
			printf("Produced %d\n", item);
			for (j=1; j<thread_count; j++) {
				sem_post(&sem_array[j]);
			}
		}
		else {
			/* Consumers - wait on personal semaphore, get random number from buffer,
			signal producer semaphore if all consumers have consumed */
			sem_wait(&sem_array[my_rank]);
			printf("Thread %ld got %d\n", my_rank, item);
			pthread_mutex_lock(&mutex);
			consumed++;
			if (consumed == thread_count-1) {
				consumed = 0;
				sem_post(&sem_array[0]);
			}
			pthread_mutex_unlock(&mutex);
		}
	}
			
	return NULL;
}
