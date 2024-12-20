/**************************************************************************
 *           Portfolio Solver Wrapper For Different Solvers               *
 *************************************************************************/

#pragma once

#include <future>
#include <vector>
#include <string>
#include <memory>

// need to include this because PortfolioSolver class needs complete type
#include "solve_via_vc.h"
#include "few_common_neighbors.h"

class Graph;
class Vertex;
//class SolverViaVC;


enum class SOLVER
{
    NONE,
    BRANCH_AND_BOUND,
    CLISAT,
    VIA_VC,
    PARALLEL,
    VC_COMP,
    LMC,
    CLASSIFIER
};

class PortfolioSolver
{
//variables
public:
    bool USE_REDUCTIONS = true;

    vector<Vertex*> maximum_clique;
    vector<string> maximum_clique_s;
    int max_clique_size = -1;

    vector<_recover_unit> rec;
    unordered_map<string, state> sol;

    // used to decide which solver to execute
    int N;
    int M;
    double density;

    std::shared_ptr<Graph> G_reduced = nullptr;

private:
    bool using_comp_vc = false; // used in parallel solver
    SolverViaVC dOmega_solver;
    future<vector<string>> parallel_solver_results[3];

//functions
public:
    void run(Graph& G, SOLVER ACTIVE_SOLVER);

    void print_maximum_clique(Graph& G, SOLVER ACTIVE_SOLVER);

    std::vector<std::string> get_clique_solution_r(vector<string>& clique, unordered_map<string, state>& sol, vector<_recover_unit>& rec){
        for(auto& c : clique)
            sol[c] = CLIQUE;
        
        for(auto it = rec.rbegin(); it != rec.rend(); it++){
            (*it).resolve(sol);
        }

        std::vector<std::string> solution;
        int _sol_count = 0;
        for(auto& [v_name, s] : sol){
            if(s == CLIQUE){
                solution.push_back(v_name);
                _sol_count++;
            }
        }
        //cout << "count = " << _sol_count << "\n";
        return solution;
    }

    std::vector<std::string> resolve_reductions(std::vector<std::string>& mc);

private:
    void run_parallel_solver(Graph& G);
    void run_classifier_solver(Graph& G);
    std::vector<std::string> get_str_clique(std::vector<Vertex*>& vertices, Graph& G);

};


