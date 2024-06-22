/**************************************************************************
 *                        Portfolio Solver Wrapper                        *
 *************************************************************************/

#include <thread>

#include "graph.h"
#include "load.h"
#include "branch_and_bound.h"
#include "benchmark.h"
#include "solve.h" // only for testing
#include "solve_via_vc.h"
#include "debug_utils.h"
#include "tests.h"

#define BENCHMARK 0; //TODO: remove once benchmark.sh works
#define ACTIVE_SOLVER SOLVER::VIA_VC

int main(int argc, char**argv){

    // #ifdef BENCHMARK
    //     run_benchmark(ACTIVE_SOLVER);
    //     return 0;
    // #endif

    Graph G;
    load_graph(G);
    //test_graph_consistency(G); //TODO: remove debug
    //std::cout << "Graph with N=" + std::to_string(G.N) + " and M=" + std::to_string(G.M) + " loaded." << std::endl;

    #if DEBUG
        G.timer.start("solve");
    #endif

    SolverViaVC solver;
    vector<Vertex*> maximum_clique;
    int max_clique_size = -1;
    switch(ACTIVE_SOLVER){
        case SOLVER::VIA_VC:
            solver = SolverViaVC();
            max_clique_size = solver.solve_via_vc(G);
            //TODO: remove this debug code
            for(std::string v : solver.maximum_clique){
                    cout << v << "\n";
            }
            break;
        case SOLVER::BRANCH_AND_BOUND:
            maximum_clique = branch_and_bound_mc(G);
            max_clique_size = maximum_clique.size();
            break;
        case SOLVER::CLISAT:
            print_error("CLISAT not implemented");
            break;
        default:
            print_warning("No solver selected");
            break;
    }
    //size_t vc_size = solve_k(G);
    //vector<Vertex*> maximum_clique = branch_and_bound_mc(G);


    #if !NDEBUG
        //print_success("Found maximum clique of size " + std::to_string(max_clique_size));
    #endif

    #ifdef RELEASE
        //G.output_vc();
        //print maximum clique
        switch(ACTIVE_SOLVER){
            case SOLVER::VIA_VC:
                for(std::string v : solver.maximum_clique){
                    cout << v << "\n";
                }
                break;
            case SOLVER::BRANCH_AND_BOUND
                for(Vertex* v : maximum_clique){
                    cout << G.name_table[v->id] << "\n";
                }
                break;
            default:
                print_warning("No solver selected");
                break;
        }
    #endif

    #if DEBUG
        cout << "max clique has size " << max_clique_size << std::endl;
        G.timer.report<chrono::milliseconds>("solve", true);
    #endif
    return 0;
}