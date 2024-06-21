cmake_minimum_required(VERSION 3.10)

# Create project
set(PROJECT_NAME "mc_solver")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20) # Set the C++ standard.
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER_ID "GNU") # Use GCC for compilation.

# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra ")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Include local header files.
include_directories("${PROJECT_SOURCE_DIR}/vc-solver/reductions")
include_directories("${PROJECT_SOURCE_DIR}/vc-solver/solve")
include_directories("${PROJECT_SOURCE_DIR}/vc-solver/lowerbounds")
include_directories("${PROJECT_SOURCE_DIR}/vc-solver")

include_directories("${PROJECT_SOURCE_DIR}/clique-solver")
include_directories("${PROJECT_SOURCE_DIR}/clique-solver/orderings")
include_directories("${PROJECT_SOURCE_DIR}/clique-solver/lowerbounds")
include_directories("${PROJECT_SOURCE_DIR}/clique-solver/upperbounds")
include_directories("${PROJECT_SOURCE_DIR}/clique-solver/reductions")
include_directories("${PROJECT_SOURCE_DIR}/clique-solver/solve")
include_directories("$(PROJECT_SOURCE_DIR)/utils")

# All source files need to be listed explicitly here.
add_executable(mc_solver
    main.cpp

    utils/debug_utils.cpp
    utils/tests.cpp

    vc-solver/graph.cpp
    vc-solver/load.cpp
    vc-solver/debug.cpp
    vc-solver/operation.cpp
    vc-solver/solve/branching.cpp
    vc-solver/solve/solve.cpp
    vc-solver/reductions/deg_rules.cpp
    vc-solver/reductions/unconfined.cpp
    vc-solver/reductions/kernel.cpp
    vc-solver/lowerbounds/clique_cover.cpp
    vc-solver/benchmark.cpp

    clique-solver/branch_and_bound.cpp
    clique-solver/upperbounds/upper_bounds.cpp
    clique-solver/lowerbounds/lower_bounds.cpp
    clique-solver/reductions/color_branching.cpp
    clique-solver/reductions/k_core.cpp
    clique-solver/orderings/degeneracy_ordering.cpp
    clique-solver/solve/solve_via_vc.cpp
)

# Define the RELEASE symbol for release builds
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Benchmark>:BENCHMARK>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:RELEASE>)
