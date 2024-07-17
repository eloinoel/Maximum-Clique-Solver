#include "graph.h"
#include "load.h"
#include "branch_and_bound.h"
#include "benchmark.h"
#include "debug_utils.h"
#include "tests.h"
#include "cli_solve.h"
#include "AMTS.h"
#include "lmc_solver.h"
#include "classifier.h"
#include "k_core.h"
#include "solve.h"
#include "portfolio_solver.h"

#include <thread>

vector<string> launch_lmc(Graph& G){
    vector<Vertex*> maximum_clique = lmc(G);
    vector<string> output;
    for(Vertex* v: maximum_clique){
        output.push_back(G.name_table[v->id]);
    }
    return output;
}

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
    //int edges = G.E.size();
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

void PortfolioSolver::run(Graph& G, SOLVER ACTIVE_SOLVER)
{
    N = G.V.size();
    M = G.E.size();
    density =  (double) (2*M)/(N * (N-1));

    #if DEBUG
        G.timer.start("solve");
    #endif

    switch(ACTIVE_SOLVER) {
        case SOLVER::LMC:
            maximum_clique = lmc(G);
            break;
        case SOLVER::VIA_VC:
            dOmega_solver = SolverViaVC();
            max_clique_size = dOmega_solver.solve_via_vc(G);
            break;
        case SOLVER::BRANCH_AND_BOUND:
            maximum_clique = branch_and_bound_mc(G);
            max_clique_size = maximum_clique.size();
            break;
        case SOLVER::CLISAT:
            solve_clique(G);
            break;
        case SOLVER::PARALLEL:
            run_parallel_solver(G);
            break;
        case SOLVER::VC_COMP:
        {
            Graph H = G.complementary_graph(G);
            cout << solve(H) << "\n";
            exit(0);
            break;
        }
        case SOLVER::CLASSIFIER:
            run_classifier_solver(G);
            break;
        default:
            print_warning("No solver selected");
            break;
    }

    #if DEBUG
        G.timer.report<chrono::milliseconds>("solve", true);
    #endif
}

void PortfolioSolver::print_maximum_clique(Graph& G, SOLVER ACTIVE_SOLVER)
{
    //print maximum clique
    switch(ACTIVE_SOLVER){
        case SOLVER::LMC:
            for(Vertex* v : maximum_clique){
                cout << G.name_table[v->id] << endl;
            }
            break;
        case SOLVER::VIA_VC:
            for(std::string v : dOmega_solver.maximum_clique){
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
                if(parallel_solver_results[0].wait_for(chrono::milliseconds(5)) == future_status::ready){
                    num = 0;
                    break;
                }
                    if(parallel_solver_results[1].wait_for(chrono::milliseconds(5)) == future_status::ready){
                    num = 1;
                    break;
                }
                if(using_comp_vc && parallel_solver_results[2].wait_for(chrono::milliseconds(5)) == future_status::ready){
                    num = 2;
                    break;
                }
            }
            auto clique_found = parallel_solver_results[num].get();
            //cout << "num" << num << " size " << clique_found.size() << "\n";
            for(string& s : clique_found)
                cout << s << "\n";
        }
            exit(EXIT_SUCCESS);
            break;
        case SOLVER::VC_COMP:
            break;
        case SOLVER::CLASSIFIER:
            break;
        case SOLVER::NONE:
            print_warning("No solver selected");
            break;
        default:
            break;
    }
}

void PortfolioSolver::run_parallel_solver(Graph& G)
{
    bool copy_domega = true;
    bool copy_cli = false; // ALL BUT LAST NEED TO BE COPIED
    if(density > 0.4 && N < 8000) {
        using_comp_vc = true;
        Graph H = G.complementary_graph(G);
        //int count = H.E.size();
        parallel_solver_results[2] = std::async(std::launch::async, launch_vc_comp, H);
    }

    if(copy_domega){
        Graph H = G.shallow_copy();
        parallel_solver_results[0] = std::async(std::launch::async, launch_dOmega, H);
    }else{
        parallel_solver_results[0] = std::async(std::launch::async, launch_dOmega, std::ref(G));
    }
    if(copy_cli){
        Graph H = G.shallow_copy();
        parallel_solver_results[1] = std::async(std::launch::async, launch_cli, std::ref(H));
    }else{
        parallel_solver_results[1] = std::async(std::launch::async, launch_cli, std::ref(G));
    }
}

void PortfolioSolver::run_classifier_solver(Graph &G)
{
    Classifier classifier;
    future<vector<string>> first_future;
    future<vector<string>> second_future;
    Graph H1;
    Expected_times prediction = classifier.predict_timeout(G);

    int first;
    int sec;

    if(prediction.LMC==NO_TIMEOUT || prediction.vc_comp==TIMEOUT){
        first = 0;
    }else first = 3;

    if(prediction.dOmega==NO_TIMEOUT)first = 2;

    sec = 1;

    H1 = G.shallow_copy();
    if(first == 0){
        first_future = std::async(std::launch::async, launch_lmc, std::ref(H1));
    }else if(first == 1){
        first_future = std::async(std::launch::async, launch_dOmega, std::ref(H1));
    }else if(first == 2){
        first_future = std::async(std::launch::async, launch_cli, std::ref(H1));
    }else{
        Graph H2 = G.complementary_graph(G);
        first_future = std::async(std::launch::async, launch_vc_comp, H2);
    }
    if(sec == 0){
        second_future = std::async(std::launch::async, launch_lmc, std::ref(G));
    }else if(sec == 1){
        second_future = std::async(std::launch::async, launch_dOmega, std::ref(G));
    }else{
        second_future = std::async(std::launch::async, launch_cli, std::ref(G));
    }


    while(true){
        if(first_future.wait_for(chrono::milliseconds(5)) == future_status::ready){
            auto clique_found = first_future.get();
            for(string s : clique_found)cout << s << "\n";
            exit(EXIT_SUCCESS);
            break;
        }
        if(second_future.wait_for(chrono::milliseconds(5)) == future_status::ready){
            auto clique_found = second_future.get();
            for(string s : clique_found)cout << s << "\n";
            exit(EXIT_SUCCESS);
            break;
        }
    }
}
