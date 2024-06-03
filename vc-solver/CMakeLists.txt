cmake_minimum_required(VERSION 3.10)

# Create project
set(PROJECT_NAME "vc")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard.
set(CMAKE_CXX_COMPILER_ID "GNU") # Use GCC for compilation.

# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra ")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

include_directories("${PROJECT_SOURCE_DIR}/include") # Include local header files.
include_directories("${PROJECT_SOURCE_DIR}/reductions")
include_directories("${PROJECT_SOURCE_DIR}/solve")
include_directories("${PROJECT_SOURCE_DIR}/lowerbounds")
include_directories("${PROJECT_SOURCE_DIR}")
# All project files need to be listed explicitly here.
add_executable(${PROJECT_NAME}
        main.cpp
        graph.cpp
        graph.h
        util.h
        load.h
        load.cpp
        debug.h 
        debug.cpp
        operation.h
        operation.cpp
        solve/branching.cpp
        solve/branching.h
        solve/solve.cpp
        solve/solve.h
        reductions/deg_rules.cpp
        reductions/deg_rules.h
        reductions/unconfined.cpp 
        reductions/unconfined.h
        reductions/kernel.cpp 
        reductions/kernel.h
        lowerbounds/clique_cover.cpp
        lowerbounds/clique_cover.h
        )
