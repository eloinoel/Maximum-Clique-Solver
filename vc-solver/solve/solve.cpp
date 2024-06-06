#include "solve.h"
#include "kernel.h"
#include "branching.h"
#include "debug.h"

#include "lowerbounds/clique_cover.h"

bool vc_branch(Graph& G){
    G.num_branches++; 

    if(G.max_degree > 0 && G.k <= 0)
        return false;

    kernelize(G);

    if(G.k < 0)
        return false;

    if(G.max_degree == 0)
        return true;

    if(basic_clique_cover(G) > G.k)
        return false;

    Vertex* branch_vertex = max_deg_branching(G);

    G.set_restore();

    G.MM_select_vertex(branch_vertex);

    if(vc_branch(G)) return true;

    G.restore();

    if(G.k < static_cast<int>(branch_vertex->degree()))
        return false;

    G.set_restore();
   
    G.MM_select_neighbors(branch_vertex);

    if(vc_branch(G)) return true;

    G.restore();

    return false;
}

size_t solve_k(Graph& G){
    G.set_restore();
    kernelize(G);
    for(size_t k = 0; k <= G.N; k++){
        G.k = k;
        G.set_restore();
        #if DEBUG
        cout << "k = " << k + G.sol_size << " | branches = " << G.num_branches  << "\r" << flush;
        #endif
        if(vc_branch(G)){
            #if DEBUG
            cout << "\n" << flush ;
            #endif
            return G.sol_size;
        }
        G.restore();
    }
    //this shouldn't be reached
    assert(false);
    return -1;
}