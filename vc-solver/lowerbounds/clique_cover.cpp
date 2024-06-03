#include "clique_cover.h"


typedef struct{
    size_t remaining = 0;
    size_t clique_size = 0;;
}clique_info;

size_t basic_clique_cover(Graph& G){
    G.new_timestamp();
    unsigned long long start = G.timestamp;

    vector<clique_info> cliques;
    for(size_t i = G.deg_lists.size() - 1; -- i != 0;){
        for(Vertex* v : G.deg_lists[i]){
            G.new_timestamp();
            auto v_iter = G.timestamp; // timestep of current iteration, speeds up checks
            // join best existing clique
            size_t best_idx = -1;

            for(Vertex* n : v->neighbors){
                if(n->marked >= start){
                    clique_info& clique = cliques[n->data.clique_data.clique_idx];

                    if(n->marked != v_iter){
                        clique.remaining = clique.clique_size;
                        n->marked = v_iter;
                    }

                    clique.remaining--;

                    if(clique.remaining == 0 && (best_idx == (size_t) -1 || clique.clique_size > cliques[best_idx].clique_size)){
                        best_idx = n->data.clique_data.clique_idx;
                    }

                }
            }
            if(best_idx != (size_t) -1){
                v->marked = v_iter;
                v->data.clique_data.clique_idx = best_idx;
                cliques[best_idx].clique_size++;
                continue;
            }
            // if none exist, create a new one
            if(v->marked < start){
                v->marked = v_iter;
                v->data.clique_data.clique_idx = cliques.size();
                cliques.push_back({1,1});
            }
        }
    }

    assert(cliques.size() <= G.N);

    return G.N - cliques.size();
}