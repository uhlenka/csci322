#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

/* Arbitrary numbers for thread_count and num_loops */
#define THREAD_COUNT 20
#define NUM_LOOPS 2

/* Global variables */
const int thread_count = THREAD_COUNT;
int item;
pthread_cond_t produce_ok, consume_ok;
pthread_mutex_t mutex;
int consumed_array[THREAD_COUNT];
int consumed = THREAD_COUNT-1;
int produced = 0;
unsigned int seed;

/* Thread function */
void* thread_work(void* rank);

/* Main */
int main(int argc, char *argv[])
{
	long thread;
	pthread_t* thread_handles;
	
	/* Initialize consumed_array */
	for (thread=0; thread<thread_count; thread++) {
		consumed_array[thread] = 0;
	}
	
	/* Initialize seed for random number generator */
	seed = time(NULL);
	
	/* Allocate space for thread_handles */
	thread_handles = malloc(thread_count*sizeof(pthread_t));
	
	/* Initialize condition variables and mutex */
	pthread_cond_init(&produce_ok, NULL);
	pthread_cond_init(&consume_ok, NULL);
	pthread_mutex_init(&mutex, NULL);
	
	/* Start threads */
	for (thread=0; thread<thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, thread_work, (void*) thread);
	}
	
	/* Join threads */
	for (thread=0; thread<thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	
	/* Destroy condition variables and mutex */
	pthread_cond_destroy(&produce_ok);
	pthread_cond_destroy(&consume_ok);
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
			/* Producer - wait on produce cond_var,
			place random number in buffer, broadcast to consume cond_var */
			pthread_mutex_lock(&mutex);
			while (consumed < thread_count-1) {
				pthread_cond_wait(&produce_ok, &mutex);
			}
			consumed = 0;
			item = rand_r(&seed)%100;
			printf("Produced %d\n", item);
			produced = 1;
			for (j=1; j<thread_count; j++) {
				consumed_array[j] = 0;
			}
			pthread_cond_broadcast(&consume_ok);
			pthread_mutex_unlock(&mutex);
		}
		else {
			/* Consumers - wait on consume cond_var, get random number from buffer,
			signal produce cond_var if all consumers have consumed */
			pthread_mutex_lock(&mutex);
			while (produced < 1 || consumed_array[my_rank] == 1) {
				pthread_cond_wait(&consume_ok, &mutex);
			}
			printf("Thread %ld got %d\n", my_rank, item);
			consumed++;
			consumed_array[my_rank] = 1;
			if (consumed == thread_count-1) {
				produced = 0;
				pthread_cond_signal(&produce_ok);
			}
			pthread_mutex_unlock(&mutex);
		}
	}
			
	return NULL;
}
