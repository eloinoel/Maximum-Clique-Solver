#include <chrono>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <fstream>
#include <set>
#include <future>

#include "benchmark.h"
#include "graph.h"
#include "k_core.h"
#include "load.h"
#include "branch_and_bound.h"
#include "upper_bounds.h"
#include "color_branching.h"
#include "degeneracy_ordering.h"
#include "solve_via_vc.h"

#define V 1
#define E 1
#define MAX_DEGREE 1
#define MIN_DEGREE 1
#define MAX_K_CORE 1 //k of max k core
#define MAX_CLIQUE 1 //size of max clique
#define TIME_BNB_BOUNDING_COLORING_KCORE 1 //time for BnB with bounding, coloring and k-core
#define TIME_BOUNDING 1 //entire time used to compute bounding
#define TIME_COLORING 1 //entire time used to compute coloring
#define TIME_KCORE 1 //entire time used to compute k-cores
#define TIME_BNB_COLORING_KCORE 1  // time for BnB with coloring and k-core
#define TIME_BNB_BOUNDING_KCORE 1 // time for BnB with bounding and k-core
#define TIME_BNB_BOUNDING_COLORING 1 //time for BnB with bounding and coloring
#define STDOUT 1 //if 0 its written

using namespace std;

static constexpr unsigned int TIMEOUT = 10; //seconds
my_time_point bnb_timeout = chrono::system_clock::now();
unordered_map<TECHNIQUE, unsigned long> time_table;

void reset_times(){
    time_table[TECHNIQUE::BRANCH_AND_BOUND] = 0;
    time_table[TECHNIQUE::BOUNDING] = 0;
    time_table[TECHNIQUE::COLORING] = 0;
    time_table[TECHNIQUE::K_CORE] = 0;
}

void add_time(TECHNIQUE technique, my_time_point start, my_time_point end){
    time_table[technique] += chrono::duration_cast<chrono::duration<double>>(end-start).count() *1e9;
}

unsigned long get_time(TECHNIQUE technique){
    return time_table[technique];
}

/**
 * @brief writes the calculated data of the benchmark into the output.csv
 *
 * @param data
 */
void add_line_to_output(vector<unsigned long> data){
    // define csv file path
    const string filePath = "output.csv";

    // create an output file
    ofstream file(filePath, ios::app);

    // write data in file
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i];
        if (i < data.size() - 1) {
            file << ",";
        }
    }
    file << "\n";

    file.close();
    return;
}
/**
 * @brief Get candidate set P
 *
 * @param G out current Graph
 * @return candidate set P
 */
vector<Vertex*> get_candidates_for_benchmark(Graph& G){
    vector<Vertex*> candidates;
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        for(Vertex* u: candidates_of_same_degree){
            candidates.push_back(u);
        }
    }

    return candidates;
}

void run_benchmark(SOLVER solver_to_execute){
    Graph G;
    load_graph(G);

    //define time_point when BnB has to cancel
    chrono::seconds offset(TIMEOUT);

    bnb_timeout = chrono::system_clock::now()+offset;
    vector<Vertex*> maximum_clique;
    unsigned long maximum_clique_size = -1;
    SolverViaVC solver;
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
            solver = SolverViaVC();
            maximum_clique_size = solver.solve_via_vc(G);
            //TODO:
            break;
        default:
            break;
    }

    int d = degeneracy(G);

    cout << G.N << ";" << G.M << ";" << G.max_degree << ";" << G.min_degree << ";" << d << ";" << maximum_clique_size << ";" << clique_core_gap(d, maximum_clique_size) << endl;
}
