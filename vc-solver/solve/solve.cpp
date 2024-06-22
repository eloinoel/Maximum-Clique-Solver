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
    if(G.old_LB >= G.UB){
        return;
    }
    G.num_branches++;
    if(kernelize(G)){
        if(G.sol_size < G.UB){
            G.set_current_vc();
            G.best_found = G.num_branches;
        }
        G.UB = min(G.sol_size, G.UB);
        return;
    }
    //auto lb = basic_clique_cover(G);
    auto lb2 = block_clique_cover(G);
    //auto lb3 = (lb2 >= G.UB)? lb2 : iterated_greedy_clique(G);
    auto lb4 = (lb2 >= G.UB)? lb2 : iterated_greedy_clique_full_permutes(G);

    
    const bool cut2  = (static_cast<int>(lb2) >= G.UB);
    const bool cut4 = (static_cast<int>(lb4) >= G.UB);
  
    if((cut2 || cut4) && G.max_degree > 0)
        return;

    if(G.max_degree == 0){
        if(G.sol_size < G.UB){
            G.set_current_vc();
            G.best_found = G.num_branches;
        }
        G.UB = min(G.sol_size, G.UB);
        return;
    }

    Vertex* branch_vertex = min_amongN_branching(G);
    
    vector<Vertex*> mirrors; 
    if(USE_MIRROR)
        mirrors = find_mirrors(branch_vertex, G); 
    
   
    G.old_LB = max(G.old_LB, lb4);
    int old_lb = G.old_LB;

    if(G.sol_size + (mirrors.size() + 1) < G.UB){
        G.set_restore();
        if(USE_PACK)
            add_selected_constraint(G, branch_vertex, 1);
        G.MM_vc_add_vertex(branch_vertex);
        for(Vertex* m : mirrors){
            G.MM_vc_add_vertex(m);   
        }
        branch(G);
        G.restore();
    }

    G.old_LB = old_lb; // max of bounds, currently only lb3 necessary

    if(G.old_LB < G.UB && ((G.sol_size + deg(branch_vertex)) < G.UB)){
        G.set_restore();
        /*if(USE_PACK && USE_MIRROR && mirrors.size() == 0)
            add_nomirror_constraint(G, branch_vertex); */
        if(USE_PACK)
            add_not_selected_constraint(G, branch_vertex);
            
        G.MM_vc_add_neighbors(branch_vertex);
        branch(G);
        G.restore();
    }
}

int solve(Graph& G){
    G.set_restore();
    kernelize(G);
    
    if(G.max_degree == 0){
        G.set_current_vc();
        return G.partial.size();
    }

    G.UB = G.N + G.sol_size;
    branch(G);
    
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