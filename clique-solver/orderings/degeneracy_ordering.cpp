#include "degeneracy_ordering.h"
#include "graph.h"

/**
 * assumes that G.max_degree > 0
 * delete this method, if min_degree is maintained in graph
 */ //TODO: PROBABLY OBSOLETE NOW
// pair<Vertex*, int> get_lowest_degree_vertex(Graph& G, int search_start_index = 0)
// {
//     while(search_start_index <= G.max_degree)
//     {
//         if(!G.deg_lists[search_start_index].empty())
//             return make_pair(G.deg_lists[search_start_index][0], search_start_index);
//         search_start_index++;
//     }
// }

/** O(E + V), source: https://dl.acm.org/doi/pdf/10.1145/2402.322385
 * Degeneracy ordering is also a minimum degree ordering.
 *
 */
vector<Vertex*> degeneracy_ordering(Graph& G)
{
    G.set_restore();

    vector<Vertex*> ordering = vector<Vertex*>();
    while(G.max_degree > 0)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        ordering.push_back(lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
    }
    G.restore();
    return ordering;
}
