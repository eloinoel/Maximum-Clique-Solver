#include "graph.h"
#include "load.h"
#include "debug.h"
#include "solve/solve.h"
#include "../clique-solver/branch_and_bound.h"

int main(int argc, char**argv){
    Graph G;
    load_graph(G);
    G.timer.start("solve");
    int max_clique_size = branch_and_bound(G);
    cout << "max clique has size " << max_clique_size;
    G.timer.report<chrono::milliseconds>("solve", true);
    return 0;
}