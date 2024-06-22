#include "degeneracy_ordering.h"


vector<Vertex*> cli_degeneracy_ordering(Graph& G){
    G.set_restore();
    vector<Vertex*> ordering(G.N);
    auto N = G.N;
    size_t i = 0;
    while(G.N > 0){
        Vertex* v         = G.deg_lists[G.min_degree].front();
        v->order_pos      = N - ++i;
        ordering[N - i] = v;
        G.MM_discard_vertex(v);
    }

    G.restore();
    return ordering;
}