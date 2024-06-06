#include "graph.h"
#include "load.h"
#include "debug.h"
#include "solve.h"

int main(int argc, char**argv){
    Graph G;
    load_graph(G);
    G.timer.start("solve");
    size_t vc_num = solve_k(G);
    cout << "vc: " << vc_num << " | branches: " << G.num_branches << "\n";
    G.timer.report<chrono::milliseconds>("solve", true);
    return 0;
}