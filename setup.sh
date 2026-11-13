#!/bin/bash

git clone https://github.com/google/benchmark.git

cd benchmark

mkdir -p build
cd build

cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
sudo cmake --build . --config Release --target install

cd ..
cd ..

# Using QuEST simulator
git clone https://github.com/QuEST-Kit/QuEST.git

# build QuEST
cd QuEST
mkdir -p build
cd build
cmake ..
cmake --build .

cd ..
cd ..