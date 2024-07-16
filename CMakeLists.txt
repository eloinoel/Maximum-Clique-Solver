cmake_minimum_required(VERSION 3.10)

# Create project
set(PROJECT_NAME "mc_solver")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard.
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER_ID "GNU") # Use GCC for compilation.

# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -pg -O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Include local header files.
# include_directories("${PROJECT_SOURCE_DIR}/vc-solver/reductions")
# include_directories("${PROJECT_SOURCE_DIR}/vc-solver/solve")
# include_directories("${PROJECT_SOURCE_DIR}/vc-solver/lowerbounds")
# include_directories("${PROJECT_SOURCE_DIR}/vc-solver/packing")
# include_directories("${PROJECT_SOURCE_DIR}/vc-solver")

# include_directories("${PROJECT_SOURCE_DIR}/clique-solver")
# include_directories("${PROJECT_SOURCE_DIR}/clique-solver/orderings")
# include_directories("${PROJECT_SOURCE_DIR}/clique-solver/lowerbounds")
# include_directories("${PROJECT_SOURCE_DIR}/clique-solver/upperbounds")
# include_directories("${PROJECT_SOURCE_DIR}/clique-solver/reductions")
# include_directories("${PROJECT_SOURCE_DIR}/clique-solver/solve")

# include_directories("${PROJECT_SOURCE_DIR}/clisat/filtering")
# include_directories("${PROJECT_SOURCE_DIR}/clisat/pruning")
# include_directories("${PROJECT_SOURCE_DIR}/clisat/partial_max_sat")
# include_directories("${PROJECT_SOURCE_DIR}/clisat/ordering")
# include_directories("${PROJECT_SOURCE_DIR}/clisat/heuristic")
# include_directories("${PROJECT_SOURCE_DIR}/clisat")

# include_directories("${PROJECT_SOURCE_DIR}/lmc/branch_reductions")
# include_directories("${PROJECT_SOURCE_DIR}/lmc")

# include_directories("$(PROJECT_SOURCE_DIR)/utils")
# include_directories("$(PROJECT_SOURCE_DIR)/portfolio-solver")

# Automatically include all directories with header files
file(GLOB_RECURSE HEADER_DIRS RELATIVE ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/*.h")

foreach(HEADER_FILE ${HEADER_DIRS})
    get_filename_component(DIR ${HEADER_FILE} PATH)
    list(APPEND INCLUDE_DIRS ${DIR})
endforeach()

list(REMOVE_DUPLICATES INCLUDE_DIRS)

foreach(DIR ${INCLUDE_DIRS})
    include_directories("${PROJECT_SOURCE_DIR}/${DIR}")
endforeach()


# All source files need to be listed explicitly here.
add_executable(mc_solver
    main.cpp
    portfolio-solver/portfolio_solver.cpp

    utils/debug_utils.cpp
    utils/tests.cpp
    utils/benchmark.cpp
    utils/classifier.cpp

    vc-solver/graph.cpp
    vc-solver/load.cpp
    vc-solver/debug.cpp
    vc-solver/operation.cpp
    vc-solver/solve/branching.cpp
    vc-solver/solve/solve.cpp
    vc-solver/reductions/deg_rules.cpp
    vc-solver/reductions/unconfined.cpp
    vc-solver/reductions/kernel.cpp
    vc-solver/reductions/desk.cpp
    vc-solver/lowerbounds/clique_cover.cpp
    vc-solver/packing/packing.cpp

    clique-solver/upperbounds/upper_bounds.cpp
    clique-solver/lowerbounds/lower_bounds.cpp
    clique-solver/reductions/k_core.cpp
    clique-solver/orderings/degeneracy_ordering.cpp
    clique-solver/solve/solve_via_vc.cpp
    clique-solver/solve/bucket_sort.cpp
    clique-solver/solve/rn.cpp
    clique-solver/solve/branch_and_bound.cpp

    lmc/lmc_solver.cpp
    lmc/branch_reductions/color_branching.cpp
    lmc/branch_reductions/inc_max_sat.cpp

    clisat/partial_max_sat/partial_max_sat.cpp
    clisat/cli_solve.cpp
    clisat/find_max_clique.cpp
    clisat/pruning/ISEQ.cpp
    clisat/ordering/degeneracy_ordering.cpp
    clisat/filtering/FILTCOL.cpp
    clisat/filtering/FILTSAT.cpp
    clisat/heuristic/AMTS.cpp
)

# Define the RELEASE symbol for release builds
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Benchmark>:BENCHMARK>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:RELEASE>)
