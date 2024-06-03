#include "kernel.h"

void kernelize(Graph& G){
    bool reduced = true;
    while(reduced){
        reduced = false;
        reduced |= deg1_rule(G);
        reduced |= deg2_rule(G);
        reduced |= unconfined_rule(G);
    }
}