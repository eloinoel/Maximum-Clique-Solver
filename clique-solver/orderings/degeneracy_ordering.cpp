#include "degeneracy_ordering.h"
#include "graph.h"

/**
 * assumes that G.max_degree > 0
 * delete this method, if min_degree is maintained in graph
 */
pair<Vertex*, int> get_lowest_degree_vertex(Graph& G, int search_start_index = 0)
{
    while(search_start_index <= G.max_degree)
    {
        if(!G.deg_lists[search_start_index].empty())
            return make_pair(G.deg_lists[search_start_index][0], search_start_index);
        search_start_index++;
    }
}

/** O(E + V), source: https://dl.acm.org/doi/pdf/10.1145/2402.322385
 * Degeneracy ordering is also a minimum degree ordering.
 *
 */
vector<Vertex*> degeneracy_ordering(Graph& G)
{
    G.set_restore();

    vector<Vertex*> ordering = vector<Vertex*>();
    int lowest_degree_index = 0;
    Vertex* lowest_degree_vertex = nullptr;
    while(!G.max_degree > 0)
    {
        //TODO: use G.min_degree instead
        std::tie(lowest_degree_vertex, lowest_degree_index) = get_lowest_degree_vertex(G, lowest_degree_index);
        ordering.push_back(lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
        // deleting a vertex can lead to other vertices being in a lower bucket
        if(lowest_degree_index > 0)
            lowest_degree_index--;
    }
    G.restore();
}
