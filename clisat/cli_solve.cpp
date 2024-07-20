#include "cli_solve.h"
#include "graph.h"
#include "lower_bounds.h"
#include "k_core.h"
#include "degeneracy_ordering.h"


vector<Vertex*> solve_clique(Graph& G, bool output){

    /*std::vector<Vertex*> degeneracy_ordering;
    std::vector<std::vector<Vertex*>> right_neighbourhoods;
    int d;
    auto result = degeneracy_ordering_rN(G);
    degeneracy_ordering = move(result.first);
    right_neighbourhoods = move(result.second);
    d = degeneracy(degeneracy_ordering, right_neighbourhoods);

    auto LB_maximum_clique = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoods);
    int clique_LB = (int) LB_maximum_clique.size();*/

    //cout << "init Lb" << clique_LB << "\n";

    /*AMTS amts = AMTS(G);
    vector<Vertex*> heuristic_clique;
    auto start = chrono::high_resolution_clock::now();
    
    bool found_once = false;
    int max_ms = 100;
    for(int k_lb = clique_LB; k_lb < G.N; k_lb++){
        bool found = amts.execute_timed(G, k_lb, start, max_ms);
        if(found){
            clique_LB = k_lb;
            heuristic_clique = amts.S_star;
            //cout << amts.S_star.size() << "\n";
            found_once = true;
        }
        else{
            break;
        }

        if(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() > max_ms)
            break;
        

    }
   
    apply_k_core(G, clique_LB);*/
    
    if(G.N == 0){return {};}
    
    auto order = cli_degeneracy_ordering(G);
  
    /*G.V = order;
    for(size_t i = 0; i < G.N; i++){
        G.V[i]->v_idx = i;
    }*/

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

    
    //cout << "LB" <<  clique_LB << "\n";
    /*if(found_once){
        //G.partial = move(heuristic_clique);
        G.best_sol = move(heuristic_clique);
        G.LB = G.best_sol.size();
    }
    else{
        G.LB = 1;
        clique_LB = 1;
    }*/

    //cout << "G LB" << G.LB << "\n";
    

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

    return G.best_sol;

   // #if !NDEBUG
    /*if(output){
        for(Vertex* v : G.best_sol)
            cout << G.name_table[v->id]<< "\n";
    }*/
    //#endif
    //cout << "max clique size = " << G.LB << ", recursive steps = " << G.num_branches << "\n";
}

vector<string> get_cli_output(Graph& G){
    vector<string> out;
    for(Vertex* v : G.best_sol)
        out.push_back(G.name_table[v->id]);
    return out;
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