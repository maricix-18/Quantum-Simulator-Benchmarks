#!/bin/bash

rm -rf build 

mkdir build
cd build
cmake ..
cmake --build .

values=(48) # run with max threads eg 48
for i in "${values[@]}"
do
    # For QuEST OMP variable, set the number of threads and CPU affinity
    # Bind threads to cores
    export VERBOSE=1
    export OMP_NUM_THREADS=$i
    export GOMP_CPU_AFFINITY="0-$((i - 1))"
    export OMP_DISPLAY_ENV=verbose
    export OMP_DISPLAY_AFFINITY=true
    
    # Confirm the environment variables are set
    echo "VERBOSE = $VERBOSE"
    echo "OMP_NUM_THREADS = $OMP_NUM_THREADS"
    echo "OMP_DISPLAY_ENV = $OMP_DISPLAY_ENV"
    echo "OMP_DISPLAY_AFFINITY = $OMP_DISPLAY_AFFINITY"
    echo "GOMP_CPU_AFFINITY = $GOMP_CPU_AFFINITY"

    ./benchmark --benchmark_repetitions=1 --benchmark_out_format=json --benchmark_out="benchmark_results_threads_${i}.json"
    ./benchmark_RQC --benchmark_repetitions=1 --benchmark_out_format=json --benchmark_out="benchmark_RQC_results_threads_${i}.json"
done 