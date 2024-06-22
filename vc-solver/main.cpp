#include "graph.h"
#include "load.h"
#include "debug.h"
#include "solve/solve.h"
#include "kernel.h"
#include "lowerbounds/clique_cover.h"
//#include "clique/solve.h"

int main(int argc, char**argv){
    Graph G;
    G.timer.start("load");
    load_graph(G);
    G.timer.start("solve");
    size_t vc_num = solve(G);
    cout << "vc: " << vc_num << " | branches: " << G.num_branches << "\n";
    G.timer.report<chrono::milliseconds>("solve", true);
    cout << "vertices in partial: " <<  G.partial.size() <<", sol_size "<< G.UB << "\n";
    return 0;
}