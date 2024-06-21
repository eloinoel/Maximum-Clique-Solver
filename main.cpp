/**************************************************************************
 *                        Portfolio Solver Wrapper                        *
 *************************************************************************/

#include <thread>

#include "graph.h"
#include "load.h"
#include "branch_and_bound.h"
#include "solve.h" // only for testing
#include "solve_via_vc.h"
#include "debug_utils.h"
#include "tests.h"

enum class execute_dOmega { YES, NO };
enum class execute_bnb { YES, NO };

void start_solver_threads(execute_bnb bnb_flag, execute_dOmega dOmega_flag)
{
    //TODO: copy graph and start multiple solvers in multiple threads
}

int main(int argc, char**argv){
    Graph G;
    load_graph(G);
    //test_graph_consistency(G); //TODO: remove debug
    std::cout << "Graph with N=" + std::to_string(G.N) + " and M=" + std::to_string(G.M) + " loaded." << std::endl;

    // #if DEBUG
    //     G.timer.start("solve");
    // #endif

    // //size_t vc_size = solve_k(G);
    // //vector<Vertex*> maximum_clique = branch_and_bound_mc(G);
    // //SolverViaVC solver = SolverViaVC();
    // //int max_clique_size = solver.solve_via_vc(G);

    // #if !NDEBUG
    //     print_success("Found maximum clique of size " + std::to_string(max_clique_size));
    // #endif

    // #ifdef RELEASE
    //     //G.output_vc();
    //     //print maximum clique
    //     for(Vertex* v : maximum_clique){
    //         cout << G.name_table[v->id] << "\n";
    //     }
    // #endif

    // #if DEBUG
    //     cout << "max clique has size " << max_clique_size << std::endl;
    //     G.timer.report<chrono::milliseconds>("solve", true);
    // #endif
    return 0;
}