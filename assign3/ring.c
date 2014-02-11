#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char **argv)
{
	int my_v, max, min, temp1, temp2;
	double my_start, my_end, my_elapsed, global_elapsed;

	/* Initialize MPI and get basic info. */
    int comm_sz;
    int my_rank;   
	MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Get (valid) num_loops from command line. */
    if (argc < 2) {
    	if (my_rank == 0) {
        	printf("Usage: mpiexec -n num_processes ./central num_loops\n");
    	}
    	MPI_Finalize();
        exit(0);
    }
    if (isdigit(argv[1][0]) == 0) {
    	if (my_rank == 0) {
        	printf("Usage: mpiexec -n num_processes ./central num_loops\n");
        }
        MPI_Finalize();
        exit(0);
    }   
    int num_loops = strtol(argv[1], NULL, 10);
    
    /* Seed random number generator. */
    unsigned int seed = my_rank;
    
    /* Synchronize processes and get start time. */
    MPI_Barrier(MPI_COMM_WORLD);
    my_start = MPI_Wtime();
    
    while (num_loops > 0) {
    
    	/* Generate random wait time. */
    	usleep((rand_r(&seed) % 500)*1000);
    	
    	/* Generate random value. */
    	my_v = rand_r(&seed) % 100;

    	/* Calculate and distribute max and min. */
    	max = min = my_v;
    	if (comm_sz > 1) {
    		if (my_rank == 0) {
    			MPI_Send(&my_v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    			MPI_Send(&my_v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    		}
    		else {
    			MPI_Recv(&temp1, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Recv(&temp2, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			if (temp1 > max) {
    				max = temp1;
    			}
    			else if (temp1 < min) {
    				min = temp1;
    			}
    			if (temp2 > max) {
    				max = temp2;
    			}
    			else if (temp2 < min) {
    				min = temp2;
    			}
    			MPI_Send(&min, 1, MPI_INT, (my_rank+1)%comm_sz, 0, MPI_COMM_WORLD);
    			MPI_Send(&max, 1, MPI_INT, (my_rank+1)%comm_sz, 0, MPI_COMM_WORLD);
    		}
    		if (my_rank == 0) {
    			MPI_Recv(&temp1, 1, MPI_INT, comm_sz-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Recv(&temp2, 1, MPI_INT, comm_sz-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Send(&my_v, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
    			MPI_Send(&my_v, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
    		}
    		else if (my_rank == comm_sz-1) {
    			MPI_Recv(&temp1, 1, MPI_INT, comm_sz-2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Recv(&temp2, 1, MPI_INT, comm_sz-2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		}
    		else {
    			MPI_Recv(&temp1, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Recv(&temp2, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    			MPI_Send(&my_v, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
    			MPI_Send(&my_v, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
    		}
    		if (temp1 > max) {
    			max = temp1;
    		}
    		else if (temp1 < min) {
    			min = temp1;
    		}
    		if (temp2 > max) {
    			max = temp2;
    		}
    		else if (temp2 < min) {
    			min = temp2;
    		}
    	}
    	
    	/* Decrement loop counter. */
    	num_loops -= 1;
	}
    
    /* Get end time. */
    my_end = MPI_Wtime();
    
    /* Calculate elapsed time. */
    my_elapsed = my_end - my_start;
    MPI_Reduce(&my_elapsed, &global_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
    	printf("Time elapsed: %.3f seconds\n", global_elapsed);
    }
    
    /* Finalize MPI and exit. */
    MPI_Finalize();
    exit(0);
}
