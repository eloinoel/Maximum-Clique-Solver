/**************************************************************************
 *                        Portfolio Solver Wrapper                        *
 *************************************************************************/

#include <thread>

#include "graph.h"
#include "load.h"
#include "branch_and_bound.h"
#include "solve.h" // only for testing

enum class execute_dOmega { YES, NO };
enum class execute_bnb { YES, NO };

void start_solver_threads(execute_bnb bnb_flag, execute_dOmega dOmega_flag)
{
    //TODO: copy graph and start multiple solvers in multiple threads
}

int main(int argc, char**argv){
    Graph G;
    load_graph(G);

    #if DEBUG
        G.timer.start("solve");
    #endif

    //size_t vc_size = solve_k(G);
    vector<Vertex*> maximum_clique = branch_and_bound(G);

    #if RELEASE
        //G.output_vc();
        //print maximum clique
        for(Vertex* v : maximum_clique){
            cout << G.name_table[v->id] << "\n";
        }
    #endif

    #if DEBUG
        cout << "max clique has size " << max_clique_size << std::endl;
        G.timer.report<chrono::milliseconds>("solve", true);
    #endif
    return 0;
}