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
#include "AMTS.h"

#include "clique-solver/reductions/k_core.h"

#include <thread>
#include <future>

#define BENCHMARK 0 //TODO: remove once benchmark.sh works
#define ACTIVE_SOLVER SOLVER::PARALLEL


vector<string> launch_dOmega(Graph G){
    SolverViaVC solver;
    solver.solve_via_vc(G);
    return solver.maximum_clique;
}

vector<string> launch_cli(Graph& G){
    solve_clique(G, false);
    return get_cli_output(G);
}

vector<string> launch_vc_comp(Graph G){
    int edges = G.E.size();
    solve(G);
    auto in_sol = G.new_timestamp();
    for(Vertex* v : G.partial)
        v->marked = in_sol;
    
    vector<string> comp_sol;
    for(Vertex* v : G.V){
        if(v->marked != in_sol)
            comp_sol.push_back(G.name_table[v->id]);
    }
    return comp_sol;
}

int main(int argc, char**argv){

    if(BENCHMARK)
    {
        run_benchmark(ACTIVE_SOLVER);
        return 0;
    }

    Graph G;
    load_graph(G);
    const int N = G.V.size();
    const int M = G.E.size();
    double density =  (double) (2*M)/(N * (N-1));
   
   
    //test_graph_consistency(G); //TODO: remove debug
    //std::cout << "Graph with N=" + std::to_string(G.N) + " and M=" + std::to_string(G.M) + " loaded." << std::endl;

    #if DEBUG
        G.timer.start("solve");
    #endif

    SolverViaVC solver;
    vector<Vertex*> maximum_clique;
    int max_clique_size = -1;
    vector<Vertex*> tmp_mc;

    bool using_comp_vc = false;
    future<vector<string>> results[3];
    switch(ACTIVE_SOLVER){
        case SOLVER::VIA_VC:
            solver = SolverViaVC();
            max_clique_size = solver.solve_via_vc(G);
            //TODO: remove this debug code
            // for(std::string v : solver.maximum_clique){
            //     cout << v << endl;
            // }
            //print_success("Found maximum clique of size " + std::to_string(max_clique_size));
            //tmp_mc = solver.convert_vertex_list(G, solver.maximum_clique);
            //assert(solver.is_clique(tmp_mc));
            break;
        case SOLVER::BRANCH_AND_BOUND:
            maximum_clique = branch_and_bound_mc(G);
            max_clique_size = maximum_clique.size();
            //TODO: remove this debug code
            // for(Vertex* v : maximum_clique){
            //     cout << G.name_table[v->id] << endl;
            // }
            // break;
        case SOLVER::CLISAT:
            solve_clique(G);
            break;
        case SOLVER::PARALLEL:
            {
                bool copy_domega = true;
                bool copy_cli = false; // ALL BUT LAST NEED TO BE COPIED
                if(density > 0.4 && N < 8000){
                    using_comp_vc = true;
                    Graph H = G.complementary_graph(G);
                    int count = H.E.size();
                    results[2] = std::async(std::launch::async, launch_vc_comp, H);
                }

                if(copy_domega){
                    Graph H = G.shallow_copy();
                    results[0] = std::async(std::launch::async, launch_dOmega, H);
                }else{
                    results[0] = std::async(std::launch::async, launch_dOmega, std::ref(G));
                }
                if(copy_cli){
                    Graph H = G.shallow_copy();
                    results[1] = std::async(std::launch::async, launch_cli, std::ref(H));
                }else{
                    results[1] = std::async(std::launch::async, launch_cli, std::ref(G));
                }
            }
            break;
        case SOLVER::VC_COMP:
        {
            Graph H = G.complementary_graph(G);
            cout << solve(H) << "\n";
            exit(0);
        }
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

    //#ifdef RELEASE
        //G.output_vc();
        //print maximum clique
        switch(ACTIVE_SOLVER){
            case SOLVER::VIA_VC:
                for(std::string v : solver.maximum_clique){
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
                    if(results[0].wait_for(chrono::milliseconds(5)) == future_status::ready){
                        num = 0;
                        break;
                    }
                     if(results[1].wait_for(chrono::milliseconds(5)) == future_status::ready){
                        num = 1;
                        break;
                    }
                    if(using_comp_vc && results[2].wait_for(chrono::milliseconds(5)) == future_status::ready){
                        num = 2;
                        break;
                    }
                }
                auto clique_found = results[num].get();
                //cout << "num" << num << " size " << clique_found.size() << "\n";
                for(string& s : clique_found)
                    cout << s << "\n";
            }   
                exit(EXIT_SUCCESS);
                break;
            default:
                print_warning("No solver selected");
                break;
        }
    //#endif

    #if DEBUG
        cout << "max clique has size " << max_clique_size << std::endl;
        G.timer.report<chrono::milliseconds>("solve", true);
    #endif
    return 0;
}
