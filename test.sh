#!/bin/bash

mkdir build 
cd build
mkdir release
cd release
cmake cmake -DCMAKE_BUILD_TYPE="release" ../..
make

file="../../benchmark/data/random/003_regular_200_20_1.dimacs"

./mc_solver < $file

echo "did it work ? 0.="
