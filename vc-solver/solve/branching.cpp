#include "branching.h"

Vertex* max_deg_branching(Graph& G){
    assert(!G.deg_lists[G.max_degree].empty());
    assert(G.max_degree > 0);
    return G.deg_lists[G.max_degree].back();
}

Vertex* min_amongN_branching(Graph& G){
    int best = numeric_limits<int>::max();
    Vertex* branching_vertex = nullptr;

    for(Vertex* v : G.deg_lists[G.max_degree]){
        G.new_timestamp();
        int among_N = 0;
        for(Vertex* n : v->neighbors){
            n->marked = G.timestamp;
        }

        for(Vertex* n : v->neighbors){
            for(Vertex* w : n->neighbors){
                among_N += (w->marked == G.timestamp);
                if(among_N >= best)
                    goto fail;
            }
        }

        best = among_N;
        branching_vertex = v;

        fail:
        continue;
    }
    return branching_vertex;
}

vector<Vertex*> find_mirrors(Vertex* v, Graph& G){
    G.new_timestamp();
    
    v->marked = G.timestamp;
    for(Vertex* n : v->neighbors){
        n->marked = G.timestamp;
    }

    vector<Vertex*> N2;
    vector<Vertex*> mirrors;

    for(Vertex* u : v->neighbors){
        for(Vertex* w : u->neighbors){
            if(w->marked != G.timestamp){
                N2.push_back(w);
                w->marked = G.timestamp;
            }
        }
    }

    for(Vertex* w: N2){
        w->marked = G.timestamp - 1;
    }
    v->marked = G.timestamp - 1;

    for(Vertex* w: N2){
        // N(v)/N(w)
        for(Vertex* x : w->neighbors){
            x->marked = G.timestamp - 1;
        }

        size_t size = 0;
        for(Vertex* u2 : v->neighbors){
            size += (u2->marked == G.timestamp);
        }

        for(Vertex* u2: v->neighbors){
            if(u2->marked != G.timestamp)
                continue;
            
            size_t count = 0;
            for(Vertex* w2 : u2->neighbors)
                count += (w2->marked == G.timestamp);

            if(count == size - 1){
                mirrors.push_back(w);
                break;
            }
                    
        }

        for(Vertex* n : v->neighbors){
            n->marked = G.timestamp;
        }
    }

    return mirrors;
}