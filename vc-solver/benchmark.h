#include <chrono>
#include <iostream>
class Graph;

using my_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
extern my_time_point bnb_timeout;

enum class TECHNIQUE{
    BRANCH_AND_BOUND,
    COLORING,
    BOUNDING,
    K_CORE
};

/**
 * @brief adds the time measured between start and end to the total measured time of the technique
 * 
 * @param technique 
 * @param start time
 * @param end time
 */
void add_time(TECHNIQUE key, my_time_point start, my_time_point end);

/**
 * @brief Get total time used for this technique 
 * 
 * @param technique 
 * @return used time [seconds]
 */
double get_time(TECHNIQUE key);

/**
 * @brief Loads the graph, runs bnb and writes to Output.csv in build/benchmark/output.csv
 * 
 */
void run_benchmark();