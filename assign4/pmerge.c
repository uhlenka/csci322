#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>
#include <math.h>


/* Comparison function for qsort. */
int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}


/* Serial mergesort for serial code time. */
void merge(int * list, int left_start, int left_end, int right_start, int right_end)
{
	/* calculate temporary list sizes */
	int left_length = left_end - left_start;
	int right_length = right_end - right_start;
 
	/* declare temporary lists */
	int left_half[left_length];
	int right_half[right_length];
 
	int r = 0; /* right_half index */
	int l = 0; /* left_half index */
	int i = 0; /* list index */
 
	/* copy left half of list into left_half */
	for (i = left_start; i < left_end; i++, l++)
	{
		left_half[l] = list[i];
	}
 
	/* copy right half of list into right_half */
	for (i = right_start; i < right_end; i++, r++)
	{
		right_half[r] = list[i];
	}
 
	/* merge left_half and right_half back into list */
	for ( i = left_start, r = 0, l = 0; l < left_length && r < right_length; i++)
	{
		if ( left_half[l] < right_half[r] ) { list[i] = left_half[l++]; }
		else { list[i] = right_half[r++]; }
	}
 
	/* Copy over leftovers of whichever temporary list hasn't finished */
	for ( ; l < left_length; i++, l++) { list[i] = left_half[l]; }
	for ( ; r < right_length; i++, r++) { list[i] = right_half[r]; }
}
void mergesort_r(int left, int right, int * list)
{
	/* Base case, the list can be no simpiler */
	if (right - left <= 1)
	{
		return;
	}
 
	/* set up bounds to slice array into */
	int left_start  = left;
	int left_end    = (left+right)/2;
	int right_start = left_end;
	int right_end   = right;
 
	/* sort left half */
	mergesort_r( left_start, left_end, list);
	/* sort right half */
	mergesort_r( right_start, right_end, list);
 
	/* merge sorted halves back together */
	merge(list, left_start, left_end, right_start, right_end);
}
void mergesort(int * list, int length)
{
	mergesort_r(0, length, list);
}


/* Main */
int main(int argc, char **argv)
{
	/* Initialize MPI and get basic info. */
    int comm_sz;
    int my_rank;   
	MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    /* Reject non-powers-of-2 number of processors. */
    if (comm_sz == 0) {
    	MPI_Finalize();
    	exit(0);
    }
    int n = comm_sz;
  	while (n != 1) {
    	if (n%2 != 0) {
    		if (my_rank == 0) {
      			printf("This program only accepts numbers of processors that are powers of 2.\n");
      		}
      		MPI_Finalize();
      		exit(0);
      	}
    	n = n/2;
  	}

    /* Get array_size from command line (must be a power of 2 and greater than or equal to num_processors). */
    if (argc < 2) {
    	if (my_rank == 0) {
        	printf("Usage: mpiexec -n num_processors ./pmerge array_size\n");
    	}
    	MPI_Finalize();
        exit(0);
    }
    if (isdigit(argv[1][0]) == 0) {
    	if (my_rank == 0) {
        	printf("Usage: mpiexec -n num_processors ./pmerge array_size\n");
        }
        MPI_Finalize();
        exit(0);
    }
    int array_size = strtol(argv[1], NULL, 10);
    if (array_size < comm_sz) {
    	if (my_rank == 0) {
        	printf("array_size must be larger than num_processors\n");
    	}
    	MPI_Finalize();
        exit(0);
    }
    n = array_size;
  	while (n != 1) {
    	if (n%2 != 0) {
    		if (my_rank == 0) {
      			printf("This program only accepts array sizes that are powers of 2.\n");
      		}
      		MPI_Finalize();
      		exit(0);
      	}
    	n = n/2;
  	}
    
    /* Seed random number generator. */
    unsigned int seed = my_rank;
    
    /* Synchronize processes and get start time. */
    double my_start, my_end, my_elapsed, global_elapsed;
    MPI_Barrier(MPI_COMM_WORLD);
    my_start = MPI_Wtime();
    
    /* Broadcast array_size to all processors. */
    if (comm_sz > 1) {
    	MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    /* Generate array_size/comm_sz numbers to fill individual arrays. */
    int merge_array[array_size], my_array[array_size], recv_array[array_size];
    int i;
    for (i=0; i<(array_size/comm_sz); i++) {
    	my_array[i] = (rand_r(&seed) % 99) + 1;
    }
    
    if (comm_sz == 1) {
    	/* Do serial mergesort. */
    	mergesort(my_array, array_size);
    	/* Print results. */
    	printf("Final sorted array:\n");
    	for (i=0; i<array_size; i++) {
    		printf(" %d", my_array[i]);
   		}
   		printf("\n");
    }
    else {
    	/* Sort individual arrays and transfer to merge_array. */
    	qsort(my_array, array_size/comm_sz, sizeof(int), cmpfunc);
    	for (i=0; i<(array_size/comm_sz); i++) {
   			merge_array[i] = my_array[i];
   		}
    
    	/* Gather individual arrays to process 0 and print. */
    	MPI_Gather(&merge_array, array_size/comm_sz, MPI_INT, &recv_array, array_size/comm_sz, MPI_INT, 0, MPI_COMM_WORLD);
   		if (my_rank == 0) {
    		printf("Individual sorted arrays:\n");
    		for (i=0; i<array_size; i++) {
    			if ((i%(array_size/comm_sz)) == 0 && i != 0) {
   					printf("\n");
   				}
    			printf(" %d", recv_array[i]);
   			}
   			printf("\n");
   		}
   	
   		/* Merge individual arrays, two at a time, into a single sorted array. */
   		double time;
   		int divide, my, recv;
   		for (time=0; time<ceil(log2((double)comm_sz)); time+=1.0) {
    		divide = (int)pow(2.0, (double)(time+1));
        	if (my_rank % divide == 0) {
        		/* Receive from process my_rank+(divide/2). */
            	MPI_Recv(&recv_array, (array_size/comm_sz)*pow(2,time), MPI_INT, my_rank+(divide/2), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            	/* Merge arrays. */
            	my = recv = 0;
            	for (i=0; i<(array_size/comm_sz)*pow(2,(time+1)); i++) {
                	if ((my >= (array_size/comm_sz)*pow(2,time) || recv_array[recv] < my_array[my]) && recv < (array_size/comm_sz)*pow(2,time)) {
                		merge_array[i] = recv_array[recv];
                		recv++;
                	}
                	else {
                		merge_array[i] = my_array[my];
                		my++;
                	}
            	}
            	/* Copy merge_array to my_array. */
            	for (i=0; i<(array_size/comm_sz)*pow(2,(time+1)); i++) {
                	my_array[i] = merge_array[i];
            	}
    		}
    		else if (my_rank % (divide/2) == 0) {
    			/* Send to process my_rank-(divide/2). */
        		MPI_Ssend(&merge_array, (array_size/comm_sz)*pow(2,time), MPI_INT, my_rank-(divide/2), 0, MPI_COMM_WORLD);
    		}
    	}
    
    	/* Print results. */
    	if (my_rank == 0) {
    		printf("Final sorted array:\n");
    		for (i=0; i<array_size; i++) {
    			printf(" %d", merge_array[i]);
   			}
   			printf("\n");
   		}
   	}
    
    /* Get end time. */
    my_end = MPI_Wtime();
    
    /* Calculate elapsed time. */
    my_elapsed = my_end - my_start;
    MPI_Reduce(&my_elapsed, &global_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
    	printf("Elapsed time: %.3f milliseconds\n", global_elapsed*1000);
    }
    
    /* Finalize MPI and exit. */
    MPI_Finalize();
    exit(0);
}
