#!/bin/bash
# ============================================================
# Benchmark script for OpenMP heat diffusion programs
# Tests various schedules (static, dynamic, guided) and chunk sizes
# across different thread counts.
# ============================================================

set -e

SRC_DIR="src"
BIN_DIR="bin"

# Compile all OpenMP versions
echo "=========================================="
echo " Compiling OpenMP versions..."
echo "=========================================="
gcc -fopenmp -O2 ${SRC_DIR}/TempGrid_HW3_omp.c -o ${BIN_DIR}/TempGrid_HW3_omp -lm
gcc -fopenmp -O2 ${SRC_DIR}/HW3_1_omp.c         -o ${BIN_DIR}/HW3_1_omp         -lm
gcc -fopenmp -O2 ${SRC_DIR}/HW3_2_omp.c         -o ${BIN_DIR}/HW3_2_omp         -lm
echo "Compilation successful."
echo ""

# Programs to benchmark
PROGRAMS=("TempGrid_HW3_omp" "HW3_1_omp" "HW3_2_omp")

# Thread counts to test
THREAD_COUNTS=(1 2 4 8)

# Schedules to test (format: "type" or "type,chunk")
SCHEDULES=(
    "static"
    "static,10"
    "static,50"
    "static,100"
    "dynamic"
    "dynamic,10"
    "dynamic,50"
    "dynamic,100"
    "guided"
    "guided,10"
    "guided,50"
    "guided,100"
)

# Output results file
RESULTS_FILE="benchmark_results.txt"
echo "Benchmark Results - $(date)" > ${RESULTS_FILE}
echo "==========================================" >> ${RESULTS_FILE}
echo "" >> ${RESULTS_FILE}

# Print header
printf "%-25s | %-8s | %-18s | %s\n" "Program" "Threads" "Schedule" "Time (sec)" | tee -a ${RESULTS_FILE}
printf "%s\n" "$(printf '%.0s-' {1..80})" | tee -a ${RESULTS_FILE}

for prog in "${PROGRAMS[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        for sched in "${SCHEDULES[@]}"; do
            # Run the program and extract the time
            output=$(OMP_NUM_THREADS=${threads} OMP_SCHEDULE="${sched}" ./${BIN_DIR}/${prog} 2>&1)

            # Extract time from the output (matches the "Time = ... (X.XXXXXXXXX sec)" pattern)
            time_sec=$(echo "${output}" | grep -oP '\(\K[0-9]+\.[0-9]+ sec' | head -1 | awk '{print $1}')

            if [ -z "${time_sec}" ]; then
                time_sec="ERROR"
            fi

            printf "%-25s | %-8s | %-18s | %s\n" "${prog}" "${threads}" "${sched}" "${time_sec}" | tee -a ${RESULTS_FILE}
        done
    done
    printf "%s\n" "$(printf '%.0s-' {1..80})" | tee -a ${RESULTS_FILE}
done

echo ""
echo "Results saved to ${RESULTS_FILE}"
