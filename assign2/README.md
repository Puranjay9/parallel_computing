# Heat Distribution Simulation using Pthreads

This repository contains a C program designed to simulate heat distribution across a 2D grid. The simulation is parallelized using POSIX threads (pthreads) to enhance performance and demonstrate the application of multithreading in computational physics problems.

## Problem Description

The program models the temporal evolution of heat distribution on a 2D grid, representing a simplified scenario of heat conduction in a square plate. The simulation uses a finite difference method to approximate the heat equation, with specific initial and boundary conditions.

### Initial and Boundary Conditions

- **Grid Size**: The computational domain is a 2D grid of size 1000 x 1000 points.
- **Initial Temperature Distribution**: 
  - All points within the central region defined by coordinates (200, 200) to (800, 800) are initialized to 500 degrees.
  - The temperature of all other points is set to zero.
- **Boundary Conditions**: Points on the boundary of the grid maintain a constant temperature of zero throughout the simulation.

### Simulation Parameters

- **Temperature Update Rule**: The temperature of a point at timestep `t` is computed based on its temperature and the temperatures of its immediate neighbors at timestep `t-1`, according to the formula:
T(x,y)(t) = T(x,y)(t-1) + Cx * (T(x+1,y)(t-1) + T(x-1,y)(t-1) – 2 * T(x,y)(t-1)) 
                      + Cy * (T(x,y+1)(t-1) + T(x,y-1)(t-1) – 2 * T(x,y)(t-1))


Where `Cx` and `Cy` are constants that govern the rate of heat transfer along the x and y axes, respectively.

- **Time Steps**: The simulation progresses through 4000 time steps.
- **Reporting**: After every 200 time steps, the program prints the temperatures at specific grid points: (1, 1), (150,150), (400, 400), (500, 500), (750, 750), and (900,900).

## Implementation Details

The program is implemented in C and utilizes pthreads to divide the computation across multiple processors, aiming for a high degree of parallel speedup. A barrier synchronization mechanism ensures that all threads have completed their calculations at each timestep before proceeding to the next, maintaining data consistency.

## Building and Running

To compile the program, use:

```bash
cc -lpthread -lrt TempGrid_HW3.c -o TempGrid_HW3
```

To run the compiled binary, specifying the number of threads:
```bash
./bin/HW3_updated <num_threads>
```
To run in batch more, comparing the performance of with different cores, you can execute the perl script below:
```bash
./automate/batch_run.pl
```

## Output
The program outputs the temperatures of specified grid points after every 200 timesteps and displays the total runtime in nanoseconds and seconds.

## License
This project is open-sourced under the MIT License.
