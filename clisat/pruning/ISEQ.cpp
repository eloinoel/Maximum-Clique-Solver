#include "ISEQ.h"


vector<vector<Vertex*>> ISEQ(Graph& G, vector<Vertex*>& ordering, int kappa){
    for(Vertex* v : ordering){
        v->data.iset.iclass = -1;
    }

    vector<vector<Vertex*>> Isets(kappa);

    for(size_t i = 0; i < kappa; i++){
        Vertex* v = ordering[i];

        if(v->data.iset.iclass != -1)
            continue;
        
        bool independent = true;
        for(Vertex* n : v->neighbors){
            if(n->data.iset.iclass == i){
                independent = false;
                break;
            }
        }

        if(independent){
            v->data.iset.iclass = i;
            Isets[i].push_back(v);
        }
    }
}

vector<vector<Vertex*>> ISEQ_sets(Graph& G, vector<Vertex*>& ordering, int subproblem_K){
    // TODO: remove lazy hack when G.V is already ordered in subgraph
    for(Vertex* v : ordering){
        v->data.iset = {-1, false};
    }

    /* <= kappa  := independent sets 
     * kappa + 1 := branching set                   
     */
    int kappa = G.LB-subproblem_K;

    vector<vector<Vertex*>> Isets(kappa + 1); 
    vector<bool> used(kappa);

    for(Vertex* v : ordering){
        for(size_t i = 0; i < used.size(); i++) 
            used[i] = false;

        for(Vertex* n : v->neighbors){
            if(n->data.iset.iclass != -1){
                used[n->data.iset.iclass] = true;
            }
        }

        size_t i = 0;
        for(bool u : used){
            if(!u)
                break;
            i++;
        }

        Isets[i].push_back(v);

        if(i < kappa)
            v->data.iset.iclass = i;
    }
    return Isets;
}


void ISEQ_color(Graph& G, vector<Vertex*>& ordering, int subproblem_K){
    for(Vertex* v : ordering){
        v->data.iset.iclass = -1;
    }

    /* <= kappa  := independent sets 
     * kappa + 1 := branching set                   
     */
    int kappa = G.LB - subproblem_K;
    //cout << "kappa = " << kappa << " coloring\n";

    vector<vector<Vertex*>> Isets(kappa + 1); 
    vector<bool> used(kappa);

    for(Vertex* v : ordering){
        for(size_t i = 0; i < used.size(); i++) 
            used[i] = false;

        for(Vertex* n : v->neighbors){
            if(n->data.iset.iclass != -1){
                used[n->data.iset.iclass] = true;
            }
        }

        size_t i = 0;
        for(bool u : used){
            if(!u)
                break;
            i++;
        }

        Isets[i].push_back(v);

        if(i < kappa)
            v->data.iset.iclass = i;
    }
}

vector<vector<Vertex*>> isets_from_colors(Graph& G, vector<Vertex*>& ordering, int kappa){
    vector<vector<Vertex*>> Isets(kappa + 1); 
    #if !NDEBUG
        vector<bool> used(kappa+1);
    #endif
    for(Vertex* v : ordering){
        // TODO: > kappa or >= kappa ? 
        if(v->data.iset.iclass == -1 )//|| v->data.iset.iclass >= kappa)
            Isets.back().push_back(v);
        else{
            Isets[v->data.iset.iclass].push_back(v);
            #if !NDEBUG
                used[v->data.iset.iclass] = true;
            #endif
        }
    }
    #if !NDEBUG
        int count = 0;
        for(bool u : used)
            count += (u == true);
    //assert(count + 1 == kappa); // if this is not the case, we can just cut the branch ?
    #endif
    return Isets;
}