#include "graph.h"
#include "load.h"
#include "debug.h"
#include "solve/solve.h"

int main(int argc, char**argv){
    Graph G;
    G.timer.start("load");
    load_graph(G);
    G.timer.report<chrono::milliseconds>("load", true);
    G.timer.start("solve");
    size_t vc_num = solve(G);
    cout << "vc: " << vc_num << " | branches: " << G.num_branches << "\n";
    G.timer.report<chrono::milliseconds>("solve", true);
    return 0;
}