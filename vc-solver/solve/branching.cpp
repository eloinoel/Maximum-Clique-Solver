#include "branching.h"

Vertex* max_deg_branching(Graph& G){
    assert(!G.deg_lists[G.max_degree].empty());
    assert(G.max_degree > 0);
    return G.deg_lists[G.max_degree].back();
}