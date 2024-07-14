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
#include "portfolio_solver.h"

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

using my_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

static constexpr unsigned int TIMEOUT = 10; //seconds
my_time_point bnb_timeout = chrono::high_resolution_clock::now();
std::unordered_map<TECHNIQUE, unsigned long> time_table;

void reset_times(){
    time_table[TECHNIQUE::BRANCH_AND_BOUND] = 0;
    time_table[TECHNIQUE::BOUNDING] = 0;
    time_table[TECHNIQUE::COLORING] = 0;
    time_table[TECHNIQUE::K_CORE] = 0;
}

void add_time(TECHNIQUE technique, my_time_point start, my_time_point end){
    time_table[technique] += std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count() *1e9;
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
    const std::string filePath = "output.csv";

    // create an output file
    std::ofstream file(filePath, std::ios::app);

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
    std::vector<Vertex*> candidates;
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        for(Vertex* u: candidates_of_same_degree){
            candidates.push_back(u);
        }
    }

    return candidates;
}

/**
 * @brief branch and bound framework with various time measurements for benchmark tests created
 *
 * The current graph has the candidate set P and our previously constructed local clique C implicitly stored
 *
 *
 * @param G our current graph
 * @param maximum_clique so far (C^*)
 */
// void branch_and_bound_for_benchmark(Graph& G, vector<Vertex*>& maximum_clique,chrono::system_clock::time_point& timeout, std::set<TECHNIQUE> config){
//     if(timeout < chrono::system_clock::now()){
//         maximum_clique.clear();
//         return;
//     }

//     chrono::system_clock::time_point start;
//     chrono::system_clock::time_point end;
//     G.num_branches++;
//     G.set_restore();

//     //if C > C^* then
//     if(G.partial.size()>maximum_clique.size()){
//         //C^* = C
//         maximum_clique= G.partial;
//     }

//     //bounding
//     if(config.contains(TECHNIQUE::BOUNDING)){
//         start = chrono::system_clock::now();
//         BOUNDING bounding = upper_bound(G, maximum_clique);
//         end = chrono::system_clock::now();
//         add_time(TECHNIQUE::BOUNDING, start, end);
//         if(bounding == CUTOFF){
//             G.restore();
//             return;
//         }
//     }

//     //reduction of candidate set P
//     if(config.contains(TECHNIQUE::K_CORE)){
//         start = chrono::system_clock::now();
//         apply_k_core(G, maximum_clique);
//         end = chrono::system_clock::now();
//         add_time(TECHNIQUE::K_CORE, start, end);
//     }

//     //reduction of branching set B
//     vector<Vertex*> branching_verticies = get_candidates_for_benchmark(G);

//     if(config.contains(TECHNIQUE::COLORING)){
//         start = chrono::system_clock::now();
//         branching_verticies = reduce_B_by_coloring(G.partial, branching_verticies, maximum_clique);
//         end = chrono::system_clock::now();
//         add_time(TECHNIQUE::COLORING, start, end);
//     }

//     //for all v ∈ B
//     while(!branching_verticies.empty()){
//         Vertex* branch_vertex = branching_verticies.back();
//         branching_verticies.pop_back();

//         G.set_restore();

//         //C' = C ∪ {v}
//         G.MM_clique_add_vertex(branch_vertex);

//         //P' = P ∩ N(v)
//         G.MM_induced_subgraph(branch_vertex->neighbors);

//         //recursive call of BnB(P', C', C^*)
//         branch_and_bound(G, maximum_clique);

//         G.restore();

//         //P = P\{v}
//         G.MM_discard_vertex(branch_vertex);

//     }

//     G.restore();
// }

void run_benchmark(SOLVER solver_to_execute){
    Graph G;
    load_graph(G);

    //define time_point when BnB has to cancel
    std::chrono::seconds offset(TIMEOUT);

    bnb_timeout = chrono::high_resolution_clock::now()+offset;
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
            //std::cout << maximum_clique_size << std::endl;
            //TODO:
            break;
        default:
            break;
    }

    int d = degeneracy(G);

    //TODO: remove this once benchmark is fixed
    std::cout << G.N << ";" << G.M << ";" << G.max_degree << ";" << G.min_degree << ";" << d << ";" << maximum_clique_size << ";" << clique_core_gap(d, maximum_clique_size) << std::endl;
    
    //fill output.csv
    // std::vector<unsigned long> times = {get_time(TECHNIQUE::BRANCH_AND_BOUND), get_time(TECHNIQUE::BOUNDING), get_time(TECHNIQUE::COLORING), get_time(TECHNIQUE::K_CORE)};
    // std::vector<unsigned long> properties = {G.N, G.M, G.max_degree, G.min_degree, max_k_core(G), maximum_clique_size};
    // add_line_to_output(times, properties);
    // //warmup
    // std::set<TECHNIQUE> config = {TECHNIQUE::BOUNDING, TECHNIQUE::COLORING, TECHNIQUE::K_CORE};
    // vector<Vertex*> maximum_clique;
    // branch_and_bound_for_benchmark(G, maximum_clique,bnb_timeout, config);
    // reset_times();


    // //run BnB with all techniques
    // config = {TECHNIQUE::BOUNDING, TECHNIQUE::COLORING, TECHNIQUE::K_CORE};
    // maximum_clique.clear();
    // auto start = chrono::system_clock::now();
    // branch_and_bound_for_benchmark(G, maximum_clique,bnb_timeout, config);
    // auto end = chrono::system_clock::now();
    // add_time(TECHNIQUE::BRANCH_AND_BOUND, start, end);
    
    // //fill data
    // std::vector<unsigned long> data {
    //     //#ifdef #endif
    //     #if V
    //         G.N,
    //     #endif
    //     #if E
    //         G.M,
    //     #endif
    //     #if MAX_DEGREE
    //         G.max_degree,
    //     #endif
    //     #if MIN_DEGREE
    //         G.min_degree,
    //     #endif
    //     #if MAX_K_CORE
    //         max_k_core(G),
    //     #endif
    //     #if MAX_CLIQUE
    //         maximum_clique.size(),
    //     #endif
    //     #if TIME_BNB_BOUNDING_COLORING_KCORE
    //         get_time(TECHNIQUE::BRANCH_AND_BOUND),
    //     #endif
    //     #if TIME_BOUNDING
    //         get_time(TECHNIQUE::BOUNDING),
    //     #endif
    //     #if TIME_COLORING
    //         get_time(TECHNIQUE::COLORING),
    //     #endif
    //     #if TIME_KCORE
    //         get_time(TECHNIQUE::K_CORE),
    //     #endif
        
    // };

    // reset_times();

    // #if TIME_BNB_COLORING_KCORE
    // //run BnB with all techniques but BOUNDING
    // config = {TECHNIQUE::COLORING, TECHNIQUE::K_CORE};
    // config = {};
    // maximum_clique.clear();
    // start = chrono::system_clock::now();
    // branch_and_bound_for_benchmark(G, maximum_clique,bnb_timeout, config);
    // end = chrono::system_clock::now();
    // add_time(TECHNIQUE::BRANCH_AND_BOUND, start, end);
    // data.push_back(get_time(TECHNIQUE::BRANCH_AND_BOUND));

    // reset_times();
    // #endif

    // #if TIME_BNB_BOUNDING_KCORE
    // //run BnB with all techniques but COLORING
    // config = {TECHNIQUE::BOUNDING, TECHNIQUE::K_CORE};
    // maximum_clique.clear();
    // start = chrono::system_clock::now();
    // branch_and_bound_for_benchmark(G, maximum_clique,bnb_timeout, config);
    // end = chrono::system_clock::now();
    // add_time(TECHNIQUE::BRANCH_AND_BOUND, start, end);
    // data.push_back(get_time(TECHNIQUE::BRANCH_AND_BOUND));

    // reset_times();
    // #endif

    // #if TIME_BNB_BOUNDING_COLORING
    // //run BnB with all techniques but COLORING
    // config = {TECHNIQUE::BOUNDING, TECHNIQUE::COLORING};
    // maximum_clique.clear();
    // start = chrono::system_clock::now();
    // branch_and_bound_for_benchmark(G, maximum_clique,bnb_timeout, config);
    // end = chrono::system_clock::now();
    // add_time(TECHNIQUE::BRANCH_AND_BOUND, start, end);
    // data.push_back(get_time(TECHNIQUE::BRANCH_AND_BOUND));

    // reset_times();
    // #endif;

    // #if STDOUT
    // for(int i=0; i<data.size(); i++){
    //     cout<<data[i];
    //     if(i<data.size()-1){
    //         cout<<",";
    //     }
    // }
    // cout<<"\n";
    // #else
    // add_line_to_output(data);
    // #endif
}
