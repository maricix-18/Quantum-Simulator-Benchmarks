# Quantum-Simulator-Benchmarks
A setup to test quantum simulator backends (supporting QuEST just now) with google benchmark library (https://github.com/google/benchmark)

Helpers:
- first setup.sh, then run.sh

- works for function in QuESTv4, if other backend the function need to be changed

- to change qubit values change 
    #define MIN_QUBITS 20
    #define MAX_QUBITS 20

- to run only one experiment in run.sh add a filter 
    ./benchmark_RQC --benchmark_filter='Benchmark_RQC_24'
