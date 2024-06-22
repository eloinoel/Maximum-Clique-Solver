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

    G.MM_vc_add_vertex(branch_vertex);

    if(vc_branch(G)) return true;

    G.restore();

    if(G.k < static_cast<int>(deg(branch_vertex)))
        return false;

    G.set_restore();
   
    G.MM_vc_add_neighbors(branch_vertex);

    if(vc_branch(G)) return true;

    G.restore();

    return false;
}

void branch(Graph& G){
    G.num_branches++;
    kernelize(G);

    if(basic_clique_cover(G) > (G.UB - G.sol_size))
        return;

    if(G.max_degree == 0){
        G.UB = min(G.sol_size, G.UB);
        return;
    }

    Vertex* branch_vertex = max_deg_branching(G);

    G.set_restore();
    G.MM_vc_add_vertex(branch_vertex);
    branch(G);
    G.restore();


    if(G.sol_size + deg(branch_vertex) <= G.UB){
        G.set_restore();
        G.MM_vc_add_neighbors(branch_vertex);
        branch(G);
        G.restore();
    }
}

int solve(Graph& G){
    G.set_restore();
    kernelize(G);
    if(G.max_degree == 0)
        return G.sol_size;
    G.UB = G.N + G.sol_size;
    branch(G);
    if(G.max_degree == 0){
        return true;
    }


    return G.UB;

}

size_t solve_k(Graph& G){
    G.set_restore();
    kernelize(G);
    for(size_t k = 0; k <= G.N; k++){
        G.k = k;
        G.set_restore();
        cout << "k = " << k + G.sol_size << " | branches = " << G.num_branches  << "\r" << flush;
        if(vc_branch(G)){ cout << "\n" << flush ;return G.sol_size;}
        G.restore();
    }
    //this shouldn't be reached
    assert(false);
    return -1;
}