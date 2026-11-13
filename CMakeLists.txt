cmake_minimum_required(VERSION 3.28)
project(benchmarks)

set(QuEST_DIR "${CMAKE_SOURCE_DIR}/QuEST")
set(QuEST_LIBRARY_DIR "${QuEST_DIR}/build")
set(QuEST_H "${QuEST_DIR}/quest/include")

find_package(benchmark REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(benchmarks mybenchmark.cc)

target_include_directories(benchmarks PUBLIC 
"${QuEST_DIR}"
"${QuEST_LIBRARY_DIR}" 
"${QuEST_H}" 
)

set_target_properties(benchmarks PROPERTIES
    BUILD_RPATH "${QuEST_LIBRARY_DIR}"
)

target_link_libraries(benchmarks
        ${QuEST_LIBRARY_DIR}/libQuEST.so
        benchmark::benchmark
        pthread
        OpenMP::OpenMP_CXX
)

add_executable(benchmark_RQC mybenchmark_RQC.cc)

target_include_directories(benchmark_RQC PUBLIC 
"${QuEST_DIR}"
"${QuEST_LIBRARY_DIR}" 
"${QuEST_H}" 
)

set_target_properties(benchmark_RQC PROPERTIES
    BUILD_RPATH "${QuEST_LIBRARY_DIR}"
)

target_link_libraries(benchmark_RQC
        ${QuEST_LIBRARY_DIR}/libQuEST.so
        benchmark::benchmark
        pthread
        OpenMP::OpenMP_CXX
)