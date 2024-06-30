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
#include "cli_solve.h"

#include <thread>
#include <future>

#define BENCHMARK 0
#define ACTIVE_SOLVER SOLVER::PARALLEL

using namespace std;

vector<string> launch_dOmega(Graph G){
    SolverViaVC solver;
    solver.solve_via_vc(G);
    return solver.maximum_clique;
}

vector<string> launch_cli(Graph& G){
    solve_clique(G, false);
    return get_cli_output(G);
}

int main(int argc, char**argv){

    if(BENCHMARK)
    {
        run_benchmark(ACTIVE_SOLVER);
        return 0;
    }

    Graph G;
    load_graph(G);

    SolverViaVC solver;
    vector<Vertex*> maximum_clique;
    int max_clique_size = -1;
    future<vector<string>> results[2];

    switch(ACTIVE_SOLVER){
        case SOLVER::VIA_VC:
            solver = SolverViaVC();
            max_clique_size = solver.solve_via_vc(G);
            break;
        case SOLVER::BRANCH_AND_BOUND:
            maximum_clique = branch_and_bound_mc(G);
            max_clique_size = maximum_clique.size();
            break;
        case SOLVER::CLISAT:
            solve_clique(G);
            break;
        case SOLVER::PARALLEL:
            {
                bool copy_domega = true;
                bool copy_cli = false; // ALL BUT LAST NEED TO BE COPIED
                if(copy_domega){
                    Graph H = G.shallow_copy();
                    results[0] = async(launch::async, launch_dOmega, H);
                }else{
                    results[0] = async(launch::async, launch_dOmega, ref(G));
                }
                if(copy_cli){
                    Graph H = G.shallow_copy();
                    results[1] = async(launch::async, launch_cli, ref(H));
                }else{
                    results[1] = async(launch::async, launch_cli, ref(G));
                }
            }
            break;
        default:
            print_warning("No solver selected");
            break;
    }

    #ifdef RELEASE
        switch(ACTIVE_SOLVER){
            case SOLVER::VIA_VC:
                for(string v : solver.maximum_clique){
                    cout << v << "\n";
                }
                break;
            case SOLVER::BRANCH_AND_BOUND:
                for(Vertex* v : maximum_clique){
                    cout << G.name_table[v->id] << endl;
                }
                break;
            case SOLVER::CLISAT:
                // NOTE: CLISAT solver prints the maximum clique in the function itself
                break;
            case SOLVER::PARALLEL:
            {
                int num = 0;
                while(true){
                    if(results[0].wait_for(chrono::milliseconds(3)) == future_status::ready){
                        num = 0;
                        break;
                    }
                     if(results[1].wait_for(chrono::milliseconds(3)) == future_status::ready){
                        num = 1;
                        break;
                    }
                }
                auto clique_found = results[num].get();
                for(string& s : clique_found)
                    cout << s << "\n";
            }
                exit(EXIT_SUCCESS);
                break;
            default:
                print_warning("No solver selected");
                break;
        }
    #endif

    return 0;
}
