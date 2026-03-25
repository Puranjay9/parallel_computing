
// (1)
// This is the baseline program for this assignment. 
// It uses iterative loops to copy the temperature values from the old grid to the new grid, and then updates the old grid with the new values.

// To run this code use: cc -lpthread -lrt TempGrid_HW3.c -o TempGrid_HW3
// to execute the binary, run ./binary <numthreads>

#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/errno.h>
#include <time.h>

// Define the grid size
#define X_SIZE 1000
#define Y_SIZE 1000

// Define the temperature constants
#define Cx 0.125
#define Cy 0.11

// Define the initial condition temperature boundary region
#define CMin 200
#define CMax 800

#define M_NPROCS 16 		// Maximum number of processors
#define TIMESTEPS 4000      // Number of timesteps
#define PRINT_STEPS 200     // Number of timesteps to print the temperature values

float old[X_SIZE][Y_SIZE];	// Current timestep temperature
float new[X_SIZE][Y_SIZE];  // Next timestep temperature
int NumThreads;

int Count;  				// Number of rows to be calculated by each thread
int Remainder;				// Remainder of the division of the number of rows by the number of threads
struct timespec StartTime;
struct timespec EndTime;

// Synchronization variables for barrier
pthread_mutex_t SyncLock;
pthread_cond_t SyncCV;
int SyncCount;

/* MG. Barrier function is used to synchronize threads. 
It is used to make sure that all threads have completed their work before moving on to the next step. 
This is crucial for maintaining data consistency across timesteps*/
void Barrier() {
	int ret;  								  // Return value for pthread functions
	pthread_mutex_lock(&SyncLock);
	SyncCount++; 							  // Increment the number of threads that have reached the barrier
	if(SyncCount == NumThreads) {
		ret = pthread_cond_broadcast(&SyncCV); // Wake up all threads
		assert(ret == 0);
		SyncCount=0; 							// Reset the count for the next barrier
	} else { 									// If not all threads have reached the barrier, wait
		ret = pthread_cond_wait(&SyncCV, &SyncLock);
		assert(ret == 0); 
	}
	pthread_mutex_unlock(&SyncLock);
}

// Function to calculate the temperature at each point in the grid by row
void* Temp(void* tmp) {
	// pass the thread id as a parameter
	long int threadId = (long int) tmp;
	int ret; 
	int j,k,block;
	// Calculate the start and end points for the thread
	int start = threadId * Count;
	// If the thread id is less than the remainder, add the thread id to the start point
	if (threadId <= Remainder)
		start += threadId;
	else
	// If the thread id is greater than the remainder, add the remainder to the start point.
		start += Remainder;
	// Calculate the end point for the thread
	int end = start + Count - 1;
	if (threadId < Remainder)
		end++;
	// Call barrier to ensure all threads start at the same time
	// Barrier(); // Commented because this is not really needed and could degrade the performance

	// Nested loop iterating through the portion of the grid assigned to the thread
	for (block = 1; block <= TIMESTEPS; block++) {	// the outer loop iterates over the timesteps
		for (j = start; j <= end; j++) { 			// this loops iterate over the rows assigned to the thread (X_SIZE is the number of rows in the grid)
			for (k = 1; k <= Y_SIZE; k++) {			// the inner loop iterates over the columns (Y_SIZE is the number of columns in the grid)
			 	
				if (j > 0 && j < X_SIZE - 1) { 		// Ensure we don't go out of bounds
				new[j][k] = (old[j][k]) + (Cx * (old[j + 1][k] + old[j - 1][k] - 2 * (old[j][k]))) + (Cy * (old[j][k + 1] + old[j][k - 1] - 2 * (old[j][k])));
				
				}
			}
		}
		Barrier();	// Ensure all threads have completed calculations
		
		// Update old grid with new values
        if (threadId == 0) { // Only one thread handles the update to avoid race conditions
            for (int x = 0; x < X_SIZE; x++) {
                for (int y = 0; y < Y_SIZE; y++) {
                    old[x][y] = new[x][y];
                }
            }
        }

		Barrier();	// Ensure all threads have completed calculations
		// Print temperatures
        if (threadId == 0 && (block % PRINT_STEPS) == 0) {
 			printf("\nThe Temperature values at points:[1,1]=%f [150,150]=%f [400,400]=%f [500,500]=%f[750,750]=%f [900,900]=%f",new[1][1],old[150][150],new[400][400],new[500][500],new[750][750],new[900][900] );
		}
	}
	return NULL;
}

// Main function
int main(int argc, char** argv) {

	 int ret;
	/*Arguments passed by command line*/
	if(argc<2) {
		fprintf(stderr, "Syntax: %s <numProcesors>\nExiting Program...\n", argv[0]);
		exit(1);
	}

	NumThreads=atoi(argv[1]);
	if (NumThreads < 1 || NumThreads >M_NPROCS) {
		fprintf(stderr,"Number of processors has to be between 1 and %d\nExiting Program...\n",M_NPROCS);
		exit(1);
	}

    // Initialize the grid
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            old[x][y] = (x >= CMin-1 && x <= CMax-1 && y >= CMin-1 && y <= CMax-1) ? 500.0 : 0.0;
        }
    }

	/*Initializing threads*/

    pthread_t threads[NumThreads];
    pthread_mutex_init(&SyncLock, NULL);
    pthread_cond_init(&SyncCV, NULL);
	assert(threads!=NULL);

	/*Step Calculation*/

	Count = (X_SIZE-1) / NumThreads;
	Remainder = (X_SIZE-1) % NumThreads;

	SyncCount=0;
    struct timespec StartTime, EndTime;
    ret = clock_gettime(CLOCK_REALTIME, &StartTime);
	assert(ret == 0);

	// Create threads
    for (long int i = 0; i < NumThreads; i++) {
        pthread_create(&threads[i], NULL, Temp, (void*)i);
    }

	// Join threads
    for (int i = 0; i < NumThreads; i++) {
        pthread_join(threads[i], NULL);
    }

	// End the clock
    clock_gettime(CLOCK_REALTIME, &EndTime);
    unsigned long long int runtime = 1000000000 * (EndTime.tv_sec - StartTime.tv_sec) + EndTime.tv_nsec - StartTime.tv_nsec;
    printf("\nTime = %llu nanoseconds (%.9f sec)\n", runtime, runtime / 1000000000.0);

	// Destroy synchronization variables
    pthread_mutex_destroy(&SyncLock);
    pthread_cond_destroy(&SyncCV);
    // free(threads); // Free dynamically allocated memory
    return 0;
}
