#include "find_max_clique.h"

#include "algorithm" // temp for sort...
#include "partial_max_sat/partial_max_sat.h"
#include "filtering/FILTCOL.h"
#include "filtering/FILTSAT.h"



void find_max_clique(Graph& G, list<Vertex*> pruned){
    G.num_branches++;
    auto in_p = G.new_timestamp();
    
    G.set_restore();

    for(size_t i = 0; i + 1 < G.V.size();i++)
        assert(G.V[i]->order_pos < G.V[i+1]->order_pos);

    //TODO: save reference coloring (efficiently?)
    int kappa = G.LB - G.partial.size();
    ISEQ_color(G, G.V, G.partial.size());
    

    vector<int> reference_coloring(G.V.size());
    vector<int> max_iset_member(kappa);
    for(size_t i = 0; i < G.V.size(); i++){
        const int c = G.V[i]->data.iset.iclass;
        reference_coloring[i] = c;
        if(c != -1)
            max_iset_member[c] = max(G.V[i]->order_pos, max_iset_member[c]);
    }

    for(Vertex* p : pruned)
        p->marked = in_p;
    pruned.clear();
    list<Vertex*> branching;
    
    // compute pruned, branching sets
    for(Vertex* v: G.V){
        //if(v->marked == in_p)
        //    continue;
        if(v->data.iset.iclass != -1){
            pruned.push_back(v);
            //v->marked = in_p;
        }else{
            //if(v->marked == in_p)
            //    continue;
            branching.push_back(v);
        }
    }

    for(Vertex* b : branching)
        assert(b->data.iset.iclass == -1);

    for(Vertex* p : pruned)
        assert(p->data.iset.iclass != -1);
    // update μ
    vector<pair<Vertex*, int>> old_mu_values;
    for(Vertex* p : pruned){
        int mu_p = 1;
        for(Vertex* n : p->neighbors){
            if(n->order_pos < p->order_pos){
                mu_p = max(mu_p, 1 + n->mu);
            }
        }
        if(p->mu != mu_p){
            old_mu_values.push_back({p, p->mu});
        }
        p->mu = mu_p;
        // mark for later - not part of the the update itself
        // p->marked = in_p;
    }

    G.MM_updated_mu(old_mu_values);

    vector<pair<Vertex*, int>> old_mu_b; // separate because pushing values one by one into history is inefficient and ugly
    old_mu_b.reserve(branching.size());

    bool partite_node = check_kappa_partite(G, branching);

    auto iter_branching = branching.begin();
    while(iter_branching != branching.end()){
        sort(G.V.begin(), G.V.end(), [](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});
        for(size_t i = 0; i + 1 < G.V.size();i++)
            assert(G.V[i]->order_pos < G.V[i+1]->order_pos);
        for(size_t i = 0; i < G.V.size(); i++){
            G.V[i]->data.iset.iclass = reference_coloring[i];
            G.V[i]->v_idx = i;
        }

        Vertex* b = *iter_branching;

        old_mu_b.push_back({b, b->mu});
        // update μ of b
        for(Vertex* n: b->neighbors){
            int mu_b = 1;
            for(Vertex* n : b->neighbors){
                if(n->order_pos < b->order_pos){
                    mu_b = max(mu_b, 1 + n->mu);
                }
            }
            b->mu = mu_b;
        }

        if(b->mu + G.partial.size() <= G.LB){
            iter_branching = branching.erase(iter_branching);
            add_to_pruned(pruned, b);
            continue;
        }
        else{
            vector<Vertex*> subproblem_V;
            /* this could be done in a single pass by increasing timestep by 2, giving all in p = old_time + 2, all in b = old_time + 1
               and checking marked - (pos_v < pos_b) >= old_time + 1  */
            in_p = G.new_timestamp(); 
            for(Vertex* p : pruned)
                p->marked = in_p;

            for(Vertex* n : b->neighbors){
                if(n->marked == in_p)
                    subproblem_V.push_back(n);
            }

            if(!partite_node){  // branch set of partite node is independent i.e. no other bj < bi will be in the set
                auto in_b = G.new_timestamp();
                for(Vertex* x: branching)
                    x->marked = in_b;

                for(Vertex* n : b->neighbors){
                    if(n->marked == in_b && (n->order_pos < b->order_pos))
                        subproblem_V.push_back(n);
                }
            }
            G.set_restore();
            G.MM_clisat_add_vertex(b);
            
            sort(subproblem_V.begin(), subproblem_V.end(), [](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});

            if(subproblem_V.empty()){ 
                //cout << "partial on empty " << G.partial.size() << "\n";
                if(G.partial.size() > G.LB){
                    G.best_sol = G.partial;
                    G.LB = G.partial.size();
                    // TODO: update clique size
                    //cout << "new best clique " << G.best_sol.size() << "\n";
                }
                G.restore();
                goto node_end;
            }

            int kappa_new = G.LB - G.partial.size(); //why? what?
            auto isets = isets_from_colors(G, subproblem_V, kappa_new);
          
            // G = G[V]
            G.MM_induced_subgraph(subproblem_V);    
            // partial κ-coloring
            // auto isets = ISEQ_sets(G, subproblem_V, G.partial.size());
            
            list<Vertex*> pruned_n;
         
            if(partite_node){
                //cout << "partite, N = " << G.V.size() << "\n";
                //exit(0);
                isets = FILTCOL(G, kappa, max_iset_member);
                FILTSAT filt = FILTSAT(isets);
                bool cut = false;
                pruned_n = filt.filter(G, cut);
                //Partial_Max_Sat out = Partial_Max_Sat(isets);
                //pruned_n = out.output_pruned_to_list();
                //cout << "filtered at " << G.num_branches << "\n";
                if(cut){
                    G.restore();
                    goto branch_end;
                }
            }
            //else{
           //     auto isets = isets_from_colors(G, subproblem_V, kappa);
           //     Partial_Max_Sat psat = Partial_Max_Sat(isets);
           //     pruned_n = psat.SATCOL(G);
            //    cout << "pruned size = " << pruned_n.size() << ", V = " << G.V.size() << "\n";
            
            //}
            else{
                Partial_Max_Sat p = Partial_Max_Sat(isets);
                pruned_n = p.output_pruned_to_list();
            }

            if(pruned_n.size() != G.V.size()){
                // recurse
                //G.MM_clisat_add_vertex(b); here?
                find_max_clique(G, pruned_n);
            }
            G.restore();
            
            // IMPORANT TO DO AFTER RESTORE BECAUSE OF PARTIAL SIZE
            b->mu = min(b->mu, (int) (G.LB - G.partial.size()));
            assert(b->mu >= 0);
        }
    branch_end:
    iter_branching++;
    }
    node_end:
    /* restore state */
    for(auto& [bv, mu_bv] : old_mu_b)
        bv->mu = mu_bv;
    G.restore();

}


void add_to_pruned(list<Vertex*>& pruned, Vertex* b){
    // fairly likely case that will skip having to go through list
    if(pruned.empty() || b->order_pos > pruned.back()->order_pos){
        pruned.push_back(b);
        return;
    }

    auto iter_p = pruned.begin();
    while(iter_p != pruned.end()){
        Vertex* p = *iter_p;
        if(p->order_pos > b->order_pos){
            pruned.insert(iter_p, b);
            return;
        }
        iter_p++;
    }
    pruned.push_back(b);
}


// check if branching set is an independent set
bool check_kappa_partite(Graph& G, list<Vertex*>& branch){
    auto in_b = G.new_timestamp();
    for(Vertex* b : branch){
        b->marked = in_b;
    }
    for(Vertex* b : branch){
        for(Vertex* n : b->neighbors){
            if(n->marked == in_b)
                return false;
        }
    }
    return true;
}