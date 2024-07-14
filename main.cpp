#include "graph.h"
#include "load.h"
#include "debug_utils.h"
#include "tests.h"
#include "benchmark.h"
#include "portfolio_solver.h"


#define BENCHMARK 0
#define ACTIVE_SOLVER SOLVER::PARALLEL


int main(int argc, char**argv){

    if(BENCHMARK)
    {
        run_benchmark(ACTIVE_SOLVER);
        return 0;
    }

    Graph G;
    load_graph(G);

    PortfolioSolver solver;
    solver.run(G, ACTIVE_SOLVER);

    #ifdef RELEASE
        solver.print_maximum_clique(G, ACTIVE_SOLVER);
    #endif

    return 0;
}
