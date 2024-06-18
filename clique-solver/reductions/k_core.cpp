#include "graph.h"

void apply_k_core(Graph &G, vector<Vertex*>& maximum_clique) {
    int to_low_degree = maximum_clique.size() - G.partial.size() - 2;
    for(int i = min(to_low_degree, (int)G.deg_lists.size()-1); i>=0 ;i--){
        while(!G.deg_lists[i].empty()){
            G.MM_discard_vertex(G.deg_lists[i].back());
        }
    }
}

void apply_k_core(Graph &G, int lowerCliqueBound)
{
    while((int) G.min_degree < lowerCliqueBound - 1)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        G.MM_discard_vertex(lowest_degree_vertex);
    }
}