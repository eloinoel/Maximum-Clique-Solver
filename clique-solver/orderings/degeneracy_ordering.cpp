#include "degeneracy_ordering.h"
#include "graph.h"

using namespace std;

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

pair<vector<Vertex*>, int> degeneracy_ordering(Graph& G)
{
    G.set_restore();

    int degeneracy = 0;
    vector<Vertex*> ordering = vector<Vertex*>();
    while(G.max_degree > 0)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        ordering.push_back(lowest_degree_vertex);

        //in our bucket graph, the degree of the current min vertex is also the rightDegree in the ordering
        int rightDegree = deg(lowest_degree_vertex);
        if(rightDegree > degeneracy)
        {
            degeneracy = rightDegree;
        }

        G.MM_discard_vertex(lowest_degree_vertex);
    }

    G.restore();
    return make_pair(ordering, degeneracy);
}
