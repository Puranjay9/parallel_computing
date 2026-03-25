// (3) 
// This is an exploration of symmerty in the temperature grid.
// it does half the work and mirrors the results to the other half.
// this minimizes the computation, but increases the overhead of mirroring the results.
// It still uses a pointer to swap the old and new grids.

// To run this code use: cc -lpthread -lrt HW3_2.c -o HW3_2
// to execute the binary, run ./HW3_2.c <numthreads>

#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#define X_SIZE 1000
#define Y_SIZE 1000
#define Cx 0.125
#define Cy 0.11
#define CMin 200
#define CMax 800
#define M_NPROCS 16
#define TIMESTEPS 4000
#define PRINT_STEPS 200

float (*old)[Y_SIZE];
float (*new)[Y_SIZE];
int NumThreads;

int Count;
int Remainder;
struct timespec StartTime;
struct timespec EndTime;

pthread_mutex_t SyncLock;
pthread_cond_t SyncCV;
int SyncCount;

void Barrier() {
    pthread_mutex_lock(&SyncLock);
    SyncCount++;
    if (SyncCount == NumThreads) {
        pthread_cond_broadcast(&SyncCV);
        SyncCount = 0;
    } else {
        pthread_cond_wait(&SyncCV, &SyncLock);
    }
    pthread_mutex_unlock(&SyncLock);
}

void* Temp(void* tmp) {
    long int threadId = (long int)tmp;
    int halfX = X_SIZE / 2; // Calculate only for the left half
    int start = threadId * (halfX - 1) / NumThreads + (threadId < Remainder ? threadId : Remainder);
    int end = start + (halfX - 1) / NumThreads - 1 + (threadId < Remainder ? 1 : 0);

    for (int block = 1; block <= TIMESTEPS; block++) {
        for (int j = start; j <= end; j++) {
            for (int k = 1; k < Y_SIZE - 1; k++) {
                if (j > 0 && j < halfX) {
                    new[j][k] = old[j][k] + Cx * (old[j + 1][k] + old[j - 1][k] - 2 * old[j][k]) + Cy * (old[j][k + 1] + old[j][k - 1] - 2 * old[j][k]);
                    // Mirror the calculated value to the right half
                    new[X_SIZE - 1 - j][k] = new[j][k];
                }
            }
        }

        Barrier();
        if (threadId == 0) {
            float (*temp)[Y_SIZE] = old;
            old = new;
            new = temp;

            if (block % PRINT_STEPS == 0) {
                printf("\nThe Temperature values at points:[1,1]=%f [150,150]=%f [400,400]=%f [500,500]=%f [750,750]=%f [900,900]=%f", old[1][1], old[150][150], old[400][400], old[500][500], old[750][750], old[900][900]);
            }
        }
        Barrier();
    }
    return NULL;
}


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Syntax: %s <numProcesors>\nExiting Program...\n", argv[0]);
        exit(1);
    }

    NumThreads = atoi(argv[1]);
    if (NumThreads < 1 || NumThreads > M_NPROCS) {
        fprintf(stderr, "Number of processors has to be between 1 and %d\nExiting Program...\n", M_NPROCS);
        exit(1);
    }

    old = malloc(X_SIZE * sizeof(*old));
    new = malloc(X_SIZE * sizeof(*new));
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            old[x][y] = (x >= CMin - 1 && x <= CMax - 1 && y >= CMin - 1 && y <= CMax - 1) ? 500.0 : 0.0;
        }
    }

    pthread_t threads[NumThreads];
    pthread_mutex_init(&SyncLock, NULL);
    pthread_cond_init(&SyncCV, NULL);

    Count = (X_SIZE - 1) / NumThreads;
    Remainder = (X_SIZE - 1) % NumThreads;

    SyncCount = 0;
    clock_gettime(CLOCK_REALTIME, &StartTime);

    for (long int i = 0; i < NumThreads; i++) {
        pthread_create(&threads[i], NULL, Temp, (void*)i);
    }

    for (int i = 0; i < NumThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_REALTIME, &EndTime);
    unsigned long long int runtime = 1000000000 * (EndTime.tv_sec - StartTime.tv_sec) + EndTime.tv_nsec - StartTime.tv_nsec;
    printf("\nTime = %llu nanoseconds (%.9f sec)\n", runtime, runtime / 1000000000.0);

    pthread_mutex_destroy(&SyncLock);
    pthread_cond_destroy(&SyncCV);
    free(old);
    free(new);

    return 0;
}