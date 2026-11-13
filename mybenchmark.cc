#include <benchmark/benchmark.h>
#include "quest.h"
#include <iostream>
#include <random>

auto min_estimator = [](const std::vector<double>& v) -> double {return *(std::min_element(std::begin(v), std::end(v)));};

#define MIN_QUBITS 20
#define MAX_QUBITS 20


// Benchmark one qubit gate
static void Benchmark_H(benchmark::State& state) {
	// Apply Hadamard on the last qubit
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
	
	for (auto _: state){
		applyHadamard(qureg, state.range(0) - 1);
	}
	
	destroyQureg(qureg);
	state.SetLabel("H");
}
BENCHMARK(Benchmark_H)->DenseRange(MIN_QUBITS,MAX_QUBITS)->ComputeStatistics("min", min_estimator);

// Benchmark two-qubit gate
static void Benchmark_CNOT(benchmark::State& state) {
	// Apply CNOT with control first qubit and target last qubit
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
	
	for (auto _: state){
		applyControlledPauliX(qureg, 0, state.range(0) - 1);
	}
	
	destroyQureg(qureg);
	state.SetLabel("CNOT");
	
}
BENCHMARK(Benchmark_CNOT)->DenseRange(MIN_QUBITS,MAX_QUBITS)->ComputeStatistics("min", min_estimator);

// Toffoli
static void Benchmark_Toffoli(benchmark::State& state) {
	// Apply Toffoli with control (0, 1) qubits and target last qubit
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
	int controls[] = {0,1};
	for (auto _: state){
		applyMultiControlledPauliX(qureg, controls, 2, state.range(0) - 1);
	}
	
	destroyQureg(qureg);
	state.SetLabel("Toffoli");
	
}
BENCHMARK(Benchmark_Toffoli)->DenseRange(MIN_QUBITS,MAX_QUBITS)->ComputeStatistics("min", min_estimator);

// QFT - built in
static void Benchmark_QFT(benchmark::State& state) {
	// built in QFT
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
	
	for (auto _: state){
		applyFullQuantumFourierTransform(qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("QFT");
	
}
BENCHMARK(Benchmark_QFT)->DenseRange(MIN_QUBITS,MAX_QUBITS)->ComputeStatistics("min", min_estimator);

BENCHMARK_MAIN();