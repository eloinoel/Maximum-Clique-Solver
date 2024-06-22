#include "kernel.h"

void kernelize(Graph& G){
    bool reduced = true;
    while(reduced){
        if(G.max_degree == 0)
            return;
        reduced = false;
        #if !AUTO_DEG0
        deg0_rule(G);
        #endif
        reduced |= deg1_rule(G);
        //reduced |= unconfined_rule(G); //TODO: include once fixed
        reduced |= deg2_rule(G);
        reduced |= deg3_rule(G);
        //reduced |= domination_rule(G);
    }
}