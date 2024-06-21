#include <chrono>
#include <iostream>
#include <unordered_map> 
#include <thread>
#include <fstream>

#include "benchmark.h"
#include "graph.h"
#include "k_core.h"
#include "load.h"
#include "branch_and_bound.h"

using my_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

static constexpr unsigned int TIMEOUT = 10; //seconds
my_time_point bnb_timeout = chrono::system_clock::now();
std::unordered_map<TECHNIQUE, double> time_table;


void add_time(TECHNIQUE technique, my_time_point start, my_time_point end){
    time_table[technique] += std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
}

double get_time(TECHNIQUE technique){
    return time_table[technique];
}

/**
 * @brief writes the calculated data of the benchmark into the output.csv
 * 
 * @param data 
 */
void add_line_to_output(vector<double>& times, vector<unsigned long> properties){
    // define csv file path
    const std::string filePath = "output.csv";

    // create an output file
    std::ofstream file(filePath, std::ios::app);

    // write times in file
    for (size_t i = 0; i < times.size(); ++i) {
        file << times[i]<<",";
    }
    // write properties in file
    for (size_t i = 0; i < properties.size(); ++i) {
        file << properties[i];
        if (i < properties.size() - 1) {
            file << ",";
        }
    }
    file << "\n";

    file.close();
    return;
}

void run_benchmark(SOLVER solver_to_execute){
    Graph G;
    load_graph(G);

    //define time_point when BnB has to cancel
    std::chrono::seconds offset(TIMEOUT);
    bnb_timeout = chrono::system_clock::now();
    bnb_timeout += offset;
    
    vector<Vertex*> maximum_clique;
    unsigned long maximum_clique_size;
    //run solver
    switch(solver_to_execute)
    {
        case SOLVER::BRANCH_AND_BOUND:
            maximum_clique = branch_and_bound_mc(G);
            maximum_clique_size = maximum_clique.size();
            break;
        case SOLVER::CLISAT:
            //TODO:
            break;
        case SOLVER::VIA_VC:
            break;
        default:
            break;
    }
    
    //fill output.csv
    std::vector<double> times = {get_time(TECHNIQUE::BRANCH_AND_BOUND), get_time(TECHNIQUE::BOUNDING), get_time(TECHNIQUE::COLORING), get_time(TECHNIQUE::K_CORE)};
    std::vector<unsigned long> properties = {G.N, G.M, G.max_degree, G.min_degree, max_k_core(G), maximum_clique_size};
    add_line_to_output(times, properties);
}
