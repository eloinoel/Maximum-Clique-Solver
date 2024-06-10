#include "k_core.h"
#include "graph.h"

void apply_k_core(Graph &G, int lowerCliqueBound)
{
    while(G.min_degree < lowerCliqueBound - 1)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        G.MM_discard_vertex(lowest_degree_vertex);
    }
}
