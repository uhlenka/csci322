#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>


int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}


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
  	
  	int merge_array[array_size], my_array[array_size], recv_array[array_size];
    
    /* Seed random number generator. */
    unsigned int seed = my_rank;
    
    double my_start, my_end, my_elapsed, global_elapsed;
    
    /* Synchronize processes and get start time. */
    MPI_Barrier(MPI_COMM_WORLD);
    my_start = MPI_Wtime();
    
    /* Broadcast array_size to all processors. */
    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    /* Generate array_size/comm_sz numbers to fill individual arrays. */
    int i;
    for (i=0; i<(array_size/comm_sz); i++) {
    	my_array[i] = (rand_r(&seed) % 99) + 1;
    }
    
    /* Sort individual arrays. */
    qsort(my_array, array_size/comm_sz, sizeof(int), cmpfunc);
    
    /* Gather individual arrays to process 0 and print. */
    MPI_Gather(&my_array, array_size/comm_sz, MPI_INT, &recv_array, array_size/comm_sz, MPI_INT, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
    	printf("Individual arrays:\n");
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
   	int core, divide;
   	for (time=0; time<ceil(log2((double)num_cores)); time+=1.0) {
    	divide = (int)pow(2.0, (double)(time+1));
    	for (core=0;core<num_cores;core++) {
        	if (core % divide == 0) {
            	/* The following if statement ensures the correct
             	output when num_cores is not a power of 2. */
            	if (core+(divide/2) < num_cores) {
                	sprintf(out, "%c%d", 'R', core+(divide/2)); //receive from process core+(divide/2)
                	printf("%4s", out);
            	}
            	/*else {
                	printf("%4s", "");
            	}*/
        	}
        	else if (core % (divide/2) == 0) {
            	sprintf(out, "%c%d", 'S', core-(divide/2)); //send to process core-(divide/2)
            	printf("%4s", out);
        	}
        	/*else {
            	printf("%4s", "");
       		}*/
    	}
    	//merge arrays here
    }
    
    /* Get end time. */
    my_end = MPI_Wtime();
    
    /* Calculate elapsed time. */
    my_elapsed = my_end - my_start;
    MPI_Reduce(&my_elapsed, &global_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
    	printf("Elapsed time: %.3f seconds\n", global_elapsed);
    }
    
    /* Finalize MPI and exit. */
    MPI_Finalize();
    exit(0);
}
