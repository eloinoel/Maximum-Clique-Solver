#!/bin/bash

function build_failed {
  echo -e "${RED}"BUILD FAILED - No executable was created."${NC}"
  exit 1
}

cd ..

mkdir -p build/release
if ! cd build/release;
then
  build_failed
fi
cd ../..

cmake . -B build/release -DCMAKE_BUILD_TYPE=Release 
if ! cmake --build build/release; then
  build_failed
fi

cd benchmark

if [[ $# -eq 0 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver"
fi

if [[ $# -eq 1 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver" --time_limit $1
fi

if [[ $# -eq 2 ]] ; then
    python3 benchmark.py "./../build/release/mc_solver" --time_limit $1 --max_time_limit_exceeded $2
fi