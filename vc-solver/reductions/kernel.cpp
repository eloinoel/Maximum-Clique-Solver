#include "kernel.h"

bool kernelize(Graph& G){
    bool reduced = true;
    while(reduced){
        while(reduced){
                if(G.sol_size >= G.UB)
                    return true;
                //reduced = false;
                #if !AUTO_DEG0
                deg0_rule(G);
                #endif
                if(G.max_degree == 0)
                    return true;
                if(deg1_rule(G)) continue;
                if(unconfined_rule(G)) continue;

                if(USE_PACK){
                    int p = packing_rule(G);
                    if(p == -1)
                        return true;
                    if(p)
                        continue;
                }

                if(deg2_rule(G)) continue;
                //if(deg3_rule(G)) continue;
                if(desk_rule(G)) continue;
                //if(deg3_rule(G)) continue;
               
                reduced = false;
        }
        //reduced |= deg3_rule(G);
    }
    return false;
}
