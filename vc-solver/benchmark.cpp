#include <chrono>
#include <iostream>
#include <unordered_map> 
#include "benchmark.h"
#include "graph.h"
#include <fstream>
#include "../clique-solver/reductions/k_core.h"

using my_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

std::unordered_map<TECHNIQUE, double> time_table;

/**
 * @brief data container represents a line in our benchmark output
 * 
 */
struct Benchmark_line{
    double branch_and_bound_time;
    double bounding_time;
    double coloring_time;
    double k_core_time;
    unsigned long N;
    unsigned long M;
    unsigned long max_degree;
    unsigned long min_degree;
    unsigned long max_k_core;
    unsigned long max_clique;

    Benchmark_line(double bb_time, double bound_time, double color_time, double k_time,
        unsigned long n, unsigned long m, unsigned long max_deg, unsigned long min_deg,
        unsigned long max_k, unsigned long max_c
        ): 
        branch_and_bound_time(bb_time), bounding_time(bound_time), coloring_time(color_time),
        k_core_time(k_time), N(n), M(m), max_degree(max_deg), min_degree(min_deg),
        max_k_core(max_k), max_clique(max_c) {}
};

void add_time(TECHNIQUE technique, my_time_point start, my_time_point end){
    time_table[technique] += std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
}

double get_time(TECHNIQUE technique){
    return time_table[technique];
}

/**
 * @brief check if G is empty
 * 
 * @param G 
 * @return true if G is empty
 * @return false if G is not empty
 */
bool empty_graph(Graph& G){
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        for(Vertex* u: candidates_of_same_degree){
            return false;
        }
    }
    return true;
}

/**
 * @brief calculates the maximum k-core iteratively increasing from 0-core
 * 
 * @param G 
 * @return k of maximum k core
 */
unsigned long max_k_core(Graph& G){
    G.set_restore();
    unsigned long i =0;
    unsigned long max = 0;
    while(!(empty_graph(G))){
        max=i-1;
        apply_k_core(G, i);
        i++;
    }
    G.restore();
    return max;
}

/**
 * @brief writes the calculated data of the benchmark into the output.csv
 * 
 * @param data 
 */
void add_line_to_output(Benchmark_line& data){
    // Define the CSV file path
    const std::string filePath = "output.csv";

    // Create an output filestream object in append mode
    std::ofstream file(filePath, std::ios::app);

    // Check if the file was successfully opened
    if (!file.is_open()) {
        std::cerr << "Could not open the file!" << std::endl;
        return;
    }

    // Define the new data to be added
    std::vector<double> times = {data.branch_and_bound_time, data.bounding_time, data.coloring_time, data.k_core_time};
    std::vector<unsigned long> properties = {data.N, data.M, data.max_degree, data.min_degree, data.max_k_core, data.max_clique};

    // Write the times to the file
    for (size_t i = 0; i < times.size(); ++i) {
        file << times[i]<<",";
    }
    //file << "\n"; // End of row

        // Write the new data to the file
    for (size_t i = 0; i < properties.size(); ++i) {
        file << properties[i];
        if (i < properties.size() - 1) {
            file << ","; // Separate the fields with commas
        }
    }
    file << "\n"; // End of row

    // Close the file
    file.close();
    return;
}

void save_benchmark_data(Graph& G, unsigned long maximum_clique_size){
    Benchmark_line bl = Benchmark_line(
        get_time(TECHNIQUE::BRANCH_AND_BOUND),
        get_time(TECHNIQUE::BOUNDING),
        get_time(TECHNIQUE::COLORING),
        get_time(TECHNIQUE::K_CORE),
        G.N,
        G.M,
        G.max_degree,
        G.min_degree,
        max_k_core(G),
        maximum_clique_size
    );
    add_line_to_output(bl);
}