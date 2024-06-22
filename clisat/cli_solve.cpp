#include "cli_solve.h"
#include "graph.h"


void solve_clique(Graph& G){
    
    auto order = cli_degeneracy_ordering(G);

    /* add heuristic clique */

    list<Vertex*> pruned = {};// {order[0]};

    /* initialize μ */
    order[0]->mu = 1;
    for(int i = 1; i < G.N; i++){
        Vertex* vi = order[i];
        int mu_vi = 1;
        for(Vertex* n : vi->neighbors){
            if(n->order_pos < i)
                mu_vi = max(mu_vi, 1 + n->mu);
        }
        vi->mu = mu_vi;
    }


    for(int i = 1; i < G.N; i++){
        //cout << "trying " << i << ", best = "  << G.best_sol.size() << "\n";
        if(deg(order[i]) + 1 < G.LB){
            order[i]->mu = G.LB;
            continue;
        }
        G.set_restore();

        switch_to_subproblem(G, order, i); // create subproblem graph

        find_max_clique(G, pruned);

        G.restore();
        //pruned.push_back(order[i]);
        order[i]->mu = G.LB;
    }

   // #if !NDEBUG
    for(Vertex* v : G.best_sol)
        cout << G.name_table[v->id]<< "\n";
    //#endif
    //cout << "max clique size = " << G.LB << ", recursive steps = " << G.num_branches << "\n";
}


void switch_to_subproblem(Graph& G, vector<Vertex*>& order, int i){
    G.new_timestamp();
    Vertex* vi = order[i];

    for(Vertex* n : vi->neighbors){
        n->marked = G.timestamp;
    }

    vector<Vertex*> subproblem;
    for(size_t j = 0; j < i; j++){
        if(order[j]->marked == G.timestamp)
            subproblem.push_back(order[j]);
    }

    //this guy is the initial clique, not in the subproblem itself?
    //subproblem.push_back(vi); // this way the vertex set is in the correct order - might be useful
    G.MM_clisat_add_vertex(vi);  // create initial clique
    G.MM_induced_subgraph(subproblem); // we are now in the subproblem graph

}

void update_mu(){}