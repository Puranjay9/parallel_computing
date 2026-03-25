
// (1) OpenMP version of the baseline heat diffusion program.
// Uses iterative loops to copy old grid to new grid after each timestep.
//
// To compile: gcc -fopenmp -O2 TempGrid_HW3_omp.c -o TempGrid_HW3_omp -lm
// To run:     OMP_NUM_THREADS=4 ./TempGrid_HW3_omp
//             OMP_NUM_THREADS=4 OMP_SCHEDULE="static,10" ./TempGrid_HW3_omp

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Define the grid size
#define X_SIZE 1000
#define Y_SIZE 1000

// Define the temperature constants
#define Cx 0.125
#define Cy 0.11

// Define the initial condition temperature boundary region
#define CMin 200
#define CMax 800

#define TIMESTEPS 4000      // Number of timesteps
#define PRINT_STEPS 200     // Number of timesteps to print the temperature values

float old[X_SIZE][Y_SIZE];  // Current timestep temperature
float new[X_SIZE][Y_SIZE];  // Next timestep temperature

int main(int argc, char** argv) {

    // Initialize the grid
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            old[x][y] = (x >= CMin - 1 && x <= CMax - 1 && y >= CMin - 1 && y <= CMax - 1) ? 500.0 : 0.0;
        }
    }

    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }
    printf("Running with %d OpenMP threads\n", num_threads);

    double start_time = omp_get_wtime();

    // Timestep loop (serial — each timestep depends on the previous)
    for (int block = 1; block <= TIMESTEPS; block++) {

        // Parallelize the row loop for maximum parallelism
        #pragma omp parallel for schedule(runtime) collapse(2)
        for (int j = 1; j < X_SIZE - 1; j++) {
            for (int k = 1; k < Y_SIZE - 1; k++) {
                new[j][k] = old[j][k]
                    + Cx * (old[j + 1][k] + old[j - 1][k] - 2.0f * old[j][k])
                    + Cy * (old[j][k + 1] + old[j][k - 1] - 2.0f * old[j][k]);
            }
        }
        // Implicit barrier at end of parallel for

        // Copy new grid to old grid (also parallelized)
        #pragma omp parallel for schedule(runtime) collapse(2)
        for (int x = 0; x < X_SIZE; x++) {
            for (int y = 0; y < Y_SIZE; y++) {
                old[x][y] = new[x][y];
            }
        }
        // Implicit barrier at end of parallel for

        // Print temperatures at diagnostic points
        if ((block % PRINT_STEPS) == 0) {
            printf("\nThe Temperature values at points:[1,1]=%f [150,150]=%f [400,400]=%f [500,500]=%f [750,750]=%f [900,900]=%f",
                   new[1][1], old[150][150], new[400][400], new[500][500], new[750][750], new[900][900]);
        }
    }

    double end_time = omp_get_wtime();
    double runtime = end_time - start_time;
    printf("\nTime = %.0f nanoseconds (%.9f sec)\n", runtime * 1e9, runtime);

    return 0;
}
