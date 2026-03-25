
// (3) OpenMP version of the symmetry-exploiting heat diffusion program.
// Computes only the left half of the grid and mirrors to the right half.
//
// To compile: gcc -fopenmp -O2 HW3_2_omp.c -o HW3_2_omp -lm
// To run:     OMP_NUM_THREADS=4 ./HW3_2_omp
//             OMP_NUM_THREADS=4 OMP_SCHEDULE="guided,10" ./HW3_2_omp

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define X_SIZE 1000
#define Y_SIZE 1000
#define Cx 0.125
#define Cy 0.11
#define CMin 200
#define CMax 800
#define TIMESTEPS 4000
#define PRINT_STEPS 200

float (*grid_old)[Y_SIZE];
float (*grid_new)[Y_SIZE];

int main(int argc, char** argv) {

    grid_old = malloc(X_SIZE * sizeof(*grid_old));
    grid_new = malloc(X_SIZE * sizeof(*grid_new));
    if (!grid_old || !grid_new) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Initialize the grid
    for (int x = 0; x < X_SIZE; x++) {
        for (int y = 0; y < Y_SIZE; y++) {
            grid_old[x][y] = (x >= CMin - 1 && x <= CMax - 1 && y >= CMin - 1 && y <= CMax - 1) ? 500.0 : 0.0;
            grid_new[x][y] = 0.0;
        }
    }

    int halfX = X_SIZE / 2;  // Only compute left half

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

        // Parallelize over the left-half rows and all interior columns
        #pragma omp parallel for schedule(runtime) collapse(2)
        for (int j = 1; j < halfX; j++) {
            for (int k = 1; k < Y_SIZE - 1; k++) {
                grid_new[j][k] = grid_old[j][k]
                    + Cx * (grid_old[j + 1][k] + grid_old[j - 1][k] - 2.0f * grid_old[j][k])
                    + Cy * (grid_old[j][k + 1] + grid_old[j][k - 1] - 2.0f * grid_old[j][k]);
                // Mirror to the right half
                grid_new[X_SIZE - 1 - j][k] = grid_new[j][k];
            }
        }
        // Implicit barrier at end of parallel for

        // Pointer swap (single-threaded, O(1) operation)
        float (*temp)[Y_SIZE] = grid_old;
        grid_old = grid_new;
        grid_new = temp;

        // Print temperatures at diagnostic points
        if (block % PRINT_STEPS == 0) {
            printf("\nThe Temperature values at points:[1,1]=%f [150,150]=%f [400,400]=%f [500,500]=%f [750,750]=%f [900,900]=%f",
                   grid_old[1][1], grid_old[150][150], grid_old[400][400], grid_old[500][500], grid_old[750][750], grid_old[900][900]);
        }
    }

    double end_time = omp_get_wtime();
    double runtime = end_time - start_time;
    printf("\nTime = %.0f nanoseconds (%.9f sec)\n", runtime * 1e9, runtime);

    free(grid_old);
    free(grid_new);

    return 0;
}
