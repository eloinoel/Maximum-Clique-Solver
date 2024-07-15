/**************************************************************************
 *           Portfolio Solver Wrapper For Different Solvers               *
 *************************************************************************/

#pragma once

#include <future>
#include <vector>
#include <string>

// need to include this because PortfolioSolver class needs complete type
#include "solve_via_vc.h"

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
    vector<Vertex*> maximum_clique;
    int max_clique_size = -1;

    // used to decide which solver to execute
    int N;
    int M;
    double density;

private:
    bool using_comp_vc = false; // used in parallel solver
    SolverViaVC dOmega_solver;
    future<vector<string>> parallel_solver_results[3];

//functions
public:
    void run(Graph& G, SOLVER ACTIVE_SOLVER);

    void print_maximum_clique(Graph& G, SOLVER ACTIVE_SOLVER);
private:
    void run_parallel_solver(Graph& G);
    void run_classifier_solver(Graph& G);

};


