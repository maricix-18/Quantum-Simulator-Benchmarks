#include <benchmark/benchmark.h>
#include "quest.h"
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

#define MIN_QUBITS 20
#define MAX_QUBITS 20

#define M_PI 3.14159265358979323846

#define datapath "../googleSycamore_data/"

auto min_estimator = [](const vector<double>& v) -> double {return *(min_element(begin(v), end(v)));};

struct Gate {
    enum Type { RX, RY, RZ, S, CX } type;
    vector<int> qubits;  // in these experiments we consider multi-qubit controlled gates
    vector<double> params;  // angle params
};

// Parser for QASM files
double evalExpression(const string &expr) {
    string s = expr;
    size_t pos;
    while ((pos = s.find("pi")) != string::npos) {
        s.replace(pos, 2, to_string(M_PI));
    }

    double result = 1.0;
    size_t mulPos = s.find("*");
    if (mulPos != string::npos) {
        double left = stod(s.substr(0, mulPos));
        double right = stod(s.substr(mulPos + 1));
        result = left * right;
    } else {
        result = stod(s);
    }
    return result;
}

// Parse qubit index from "q[x]"
int parseQubitIndex(const string &token) {
    size_t start = token.find("[");
    size_t end = token.find("]");
    if (start == string::npos || end == string::npos || end <= start) return -1;
    return stoi(token.substr(start + 1, end - start - 1));
}

// Split string by comma
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

//parser 
vector<Gate> parse_qasm(const string& filepath)
{
	ifstream infile(filepath);
	if (!infile.is_open()) {
		cerr << "Cannot open QASM file: " << filepath << endl;
		return vector<Gate>();
	}

	string line;
	vector<Gate> gates;

	while (getline(infile, line)) {
		// Remove comments and trim
		size_t commentPos = line.find("//");
		if (commentPos != string::npos) line = line.substr(0, commentPos);
		if (line.empty()) continue;
		if (line.find("OPENQASM") != string::npos || line.find("include") != string::npos) continue;
		if (line.find("qreg") != string::npos) continue; // skip qreg line

		// Remove trailing semicolon
		if (!line.empty() && line.back() == ';') line.pop_back();

		// Identify gate and parameters
		size_t parenPos = line.find("(");
		string gate;
		string params;
		if (parenPos != string::npos) {
			gate = line.substr(0, parenPos);
			size_t closeParen = line.find(")", parenPos);
			params = line.substr(parenPos + 1, closeParen - parenPos - 1);
		} else {
			// Gate without parameters (sx, sxdg, s)
			size_t spacePos = line.find(" ");
			gate = line.substr(0, spacePos);
		}

		// Extract qubit indices (last, possibly multiple qubits)
		size_t spacePos = line.rfind(" ");
		string qubitToken = (spacePos != string::npos) ? line.substr(spacePos + 1) : "";
		vector<string> qubitsTokens = split(qubitToken, ',');

		vector<int> qubitIndices;
		for (auto &qt : qubitsTokens) {
			int idx = parseQubitIndex(qt);
			if (idx >= 0) qubitIndices.push_back(idx);
		}

		// Apply gate
		if (gate == "rx") {
			double theta = evalExpression(params);
			//applyRotateX(qureg, qubitIndices[0], theta);
            gates.push_back({Gate::Type::RX, {qubitIndices[0]}, {theta}});
            
		} else if (gate == "ry") {
			double theta = evalExpression(params);
			//applyRotateY(qureg, qubitIndices[0], theta);
            gates.push_back({Gate::Type::RY, {qubitIndices[0]}, {theta}});
		} else if (gate == "rz") {
			double theta = evalExpression(params);
			//applyRotateZ(qureg, qubitIndices[0], theta);
            gates.push_back({Gate::Type::RZ, {qubitIndices[0]}, {theta}});
		} else if (gate == "u2") {
			auto p = split(params, ',');
			double phi = evalExpression(p[0]);
			double lambda = evalExpression(p[1]);
			//applyRotateZ(qureg, qubitIndices[0], phi);
			//applyRotateY(qureg, qubitIndices[0], M_PI/2);
			//applyRotateZ(qureg, qubitIndices[0], lambda);
            gates.push_back({Gate::Type::RZ, {qubitIndices[0]}, {phi}});
            gates.push_back({Gate::Type::RY, {qubitIndices[0]}, {M_PI/2}});
            gates.push_back({Gate::Type::RZ, {qubitIndices[0]}, {lambda}});
		} else if (gate == "u3") {
			auto p = split(params, ',');
			double theta = evalExpression(p[0]);
			double phi = evalExpression(p[1]);
			double lambda = evalExpression(p[2]);
			//applyRotateZ(qureg, qubitIndices[0], phi);
			//applyRotateY(qureg, qubitIndices[0], theta);
			//applyRotateZ(qureg, qubitIndices[0], lambda);
            gates.push_back({Gate::Type::RZ, {qubitIndices[0]}, {phi}});
            gates.push_back({Gate::Type::RY, {qubitIndices[0]}, {theta}});
            gates.push_back({Gate::Type::RZ, {qubitIndices[0]}, {lambda}});
		} else if (gate == "sx") {
			//applyRotateX(qureg, qubitIndices[0], M_PI/2);
            gates.push_back({Gate::Type::RX, {qubitIndices[0]}, {M_PI/2}});
		} else if (gate == "sxdg") {
			//applyRotateX(qureg, qubitIndices[0], -M_PI/2);
            gates.push_back({Gate::Type::RX, {qubitIndices[0]}, {-M_PI/2}});
		} else if (gate == "s") {
			//applyPhaseShift(qureg, qubitIndices[0], M_PI/2);
            gates.push_back({Gate::Type::S, {qubitIndices[0]}, {M_PI/2}});
		} else if (gate == "cx") {
			//applyControlledPauliX(qureg, qubitIndices[0], qubitIndices[1]);
            gates.push_back({Gate::Type::CX, {qubitIndices[0], qubitIndices[1]}, {}});
		} else {
			cerr << "Unknown gate: " << gate << endl;
		}
	}
	infile.close();
	// finish reading 
	// return gate list
	return gates;
}

//execute gates
void apply_gates(const vector<Gate>& gates, Qureg& qureg) {
    for (const auto& g : gates) {
        switch (g.type) { 
            case Gate::Type::RX:
                applyRotateX(qureg, g.qubits[0], g.params[0]);
                break;
            case Gate::Type::RY:
                applyRotateY(qureg, g.qubits[0], g.params[0]);
                break;
            case Gate::Type::RZ:
                applyRotateZ(qureg, g.qubits[0], g.params[0]);
                break;
            case Gate::Type::S:
                applyPhaseShift(qureg, g.qubits[0], g.params[0]);
                break;
            case Gate::Type::CX:
                applyControlledPauliX(qureg, g.qubits[0], g.qubits[1]);
                break;
         }
    }
}

// RQC - Google Sycamore
static void Benchmark_RQC_24(benchmark::State& state) {
	// RQC of 24 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n24_m14_s0_e0_pEFGH.qasm");

	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_24");
	
}
BENCHMARK(Benchmark_RQC_24)->Arg(24)->ComputeStatistics("min", min_estimator);

static void Benchmark_RQC_28(benchmark::State& state) {
	// RQC of 28 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n28_m14_s0_e0_pEFGH.qasm");
	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_28");
	
}
BENCHMARK(Benchmark_RQC_28)->Arg(28)->ComputeStatistics("min", min_estimator);

static void Benchmark_RQC_32(benchmark::State& state) {
	// RQC of 32 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));
    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n32_m14_s0_e0_pEFGH.qasm");

	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_32");
	
}
BENCHMARK(Benchmark_RQC_32)->Arg(32)->ComputeStatistics("min", min_estimator);

static void Benchmark_RQC_34(benchmark::State& state) {
	// RQC of 34 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));

    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n34_m14_s0_e0_pEFGH.qasm");

	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_34");
	
}
BENCHMARK(Benchmark_RQC_34)->Arg(34)->ComputeStatistics("min", min_estimator);

static void Benchmark_RQC_36(benchmark::State& state) {
	// RQC of 36 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));

    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n36_m14_s0_e0_pEFGH.qasm");

	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_36");
	
}
BENCHMARK(Benchmark_RQC_36)->Arg(36)->ComputeStatistics("min", min_estimator);

static void Benchmark_RQC_38(benchmark::State& state) {
	// RQC of 38 from https://datadryad.org/dataset/doi:10.5061/dryad.k6t1rj8#usage
	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));

    // first parse everything then execute
    auto gates = parse_qasm(datapath "circuit_n38_m14_s0_e0_pEFGH.qasm");

	for (auto _: state){
		apply_gates(gates, qureg);
	}
	
	destroyQureg(qureg);
	state.SetLabel("RQC_38");
	
}
BENCHMARK(Benchmark_RQC_38)->Arg(38)->ComputeStatistics("min", min_estimator);
	
// RQC - Cliffords
static void Benchmark_RQC_Clifford(benchmark::State& state) {
	// Random RQC circuit made out of clifford gates
	random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> gateDist(0, 4); // 0:H, 1:S, 2:Sdg, 3:CX
    uniform_int_distribution<> qubitDist(0, state.range(0) - 1);

	if (isQuESTEnvInit() == 0)
		initQuESTEnv();
    Qureg qureg = createQureg(state.range(0));

	// consider n gates for n qubit circuits
	int ngates = state.range(0); // number of gates

	for (auto _: state){
		for (int i = 0; i < ngates; i++) {
			int g = gateDist(gen);

			if (g == 0) { // H
				int q = qubitDist(gen);
				applyHadamard(qureg, q);
				// cout << "H q[" << q << "]\n";
			}
			else if (g == 1) { // S
				int q = qubitDist(gen);
				applyPhaseShift(qureg, q, M_PI/2);
				// cout << "S q[" << q << "]\n";
			}
			else if (g == 2) { // Sdg
				int q = qubitDist(gen);
				applyPhaseShift(qureg, q, -M_PI/2);
				// cout << "Sdg q[" << q << "]\n";
			}
			else if (g == 3) { // CX
				int control = qubitDist(gen);
				int target = qubitDist(gen);
				while (target == control) target = qubitDist(gen); // avoid self-CX
				applyControlledPauliX(qureg, control, target);
				// cout << "CX q[" << control << "],q[" << target << "]\n";
			}
		}
	}
	destroyQureg(qureg);
	state.SetLabel("RQC_Clifford");
	
}
BENCHMARK(Benchmark_RQC_Clifford)->DenseRange(MIN_QUBITS,MAX_QUBITS)->ComputeStatistics("min", min_estimator);


BENCHMARK_MAIN();