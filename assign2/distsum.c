#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

void display(int time, int num_cores)
{
    int core;
    char out[4];
    int divide = (int)pow(2.0, (double)(time+1));
    
    for (core=0;core<num_cores;core++) {
        if (core % divide == 0) {
            /* The following if statement ensures the correct
             output when num_cores is not a power of 2. */
            if (core+(divide/2) < num_cores) {
                sprintf(out, "%c%d", 'R', core+(divide/2));
                printf("%4s", out);
            }
            else {
                printf("%4s", "");
            }
        }
        else if (core % (divide/2) == 0) {
            sprintf(out, "%c%d", 'S', core-(divide/2));
            printf("%4s", out);
        }
        else {
            printf("%4s", "");
        }
    }
}



int main(int argc, char **argv)
{
    /* Make sure a valid number of cores is given. */
    if (argc < 2) {
        printf("Usage: distsum num_cores\n");
        exit(0);
    }
    if (isdigit(argv[1][0]) == 0) {
        printf("Usage: distsum num_cores\n");
        exit(0);
    }
    
    /* Get the number of cores. */
    int num_cores = strtol(argv[1], NULL, 10);
    
    /* Print the first line of the table. */
    int core;
    for (core=0;core<=num_cores;core++) {
        if (core == 0) {
            printf("%1s", "");
        }
        else {
            printf("%4d", core-1);
        }
    }
    printf("\n");
    
    /* Print the rest of the table. */
    double time;
    for (time=0; time<ceil(log2((double)num_cores)); time+=1.0) {
        printf("%1d", (int)time);
        display((int)time, num_cores);
        printf("\n");
    }
    
    exit(0);
}