# Maximum Clique Solver
## Overview

Solver is developed in C++. Benchmark script and instructions on how to run it can be found under `/benchmark/`.

## Requirements:

- Development tools (compiler, dependencies, ...)

  - Ubuntu/Debian:
    ```bash
    sudo apt update
    sudo apt install build-essential
    ```

- CMake >= 3.14

  - Ubuntu/Debian:
    ```bash
    sudo apt install cmake
    ```

## Build

Execute one of the following commands:
```bash
  echo bash compile.sh --release
  echo bash compile.sh --debug
  echo bash compile.sh --profile
```

## Execute

Pipe input data into the solver, e.g.:
```bash
./build/release/mc_solver < data/000_random_10_4.dimacs
```

Or use the benchmark script found under `/benchmark/`.

## Useful commands:

##### Build & execute one instance
```bash
bash compile.sh --release && ./build/release/mc_solver < data/000_random_10_4.dimacs
```

##### Run benchmark on release build
From root directory run:
```bash
bash compile.sh --release && cd benchmark && python3 benchmark.py "./../build/release/mc_solver" && cd ..
```

## Input/Out Specification
### Input

The input should be in the following format ("edge list").
Each line contains exactly two numbers x and y in no particular order, separated by one or more whitespaces.
Each such pair of numbers constitutes an edge between x and y.
No order may be assumed on the edges and the nodes within one line.
Comments line start with # and should be ignored
The first line of the file is always a comment containing the number of nodes (n) and edges (m). The vertices are always numbers in the range [1, ..., n]

Example:
```bash
  # 10 11
  1 2
  1 4
  1 7
  1 9
  2 9
  3 1
  4 2
  4 9
  6 4
  7 2
  10 7
```
### Output

Program outputs a maximum clique in the input graph, one node per line.
Output node names correspond to the node names in the input file.

Example:
```bash
  4
  9
  1
  2
```