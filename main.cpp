#include "graph.h"
#include "load.h"
#include "debug_utils.h"
#include "tests.h"
#include "benchmark.h"
#include "portfolio_solver.h"

//TODO: delete after testing
#include "k_truss.h"
#include "./clique-solver/orderings/degeneracy_ordering.h"


#define BENCHMARK 0
#define ACTIVE_SOLVER SOLVER::PARALLEL

int main(){

    if(BENCHMARK)
    {
        run_benchmark(ACTIVE_SOLVER);
        return 0;
    }

    Graph G;
    load_graph(G);

    //TODO: delete after testing
    std::pair<std::vector<Vertex*>,std::vector<int>> ordering = degeneracy_ordering(G);
    KTruss k_truss = KTruss(G, ordering.first);

    // PortfolioSolver solver;
    // solver.run(G, ACTIVE_SOLVER);

    #ifdef RELEASE
        solver.print_maximum_clique(G, ACTIVE_SOLVER);
    #endif

    return 0;
}
