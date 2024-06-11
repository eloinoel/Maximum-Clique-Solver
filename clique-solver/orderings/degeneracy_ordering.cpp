#include "degeneracy_ordering.h"
#include "graph.h"

using namespace std;

pair<unique_ptr<vector<Vertex*>>, unique_ptr<vector<int>>> degeneracy_ordering(Graph& G)
{
    G.set_restore();

    unique_ptr<vector<Vertex*>> ordering = make_unique<vector<Vertex*>>();
    unique_ptr<vector<int>> right_degrees = make_unique<vector<int>>(G.V, -1); //init with -1

    while(G.max_degree > 0)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        ordering->push_back(lowest_degree_vertex);

        //in our bucket graph, the degree of the current min vertex is also the rightDegree in the ordering
        right_degrees->at(lowest_degree_vertex->id) = deg(lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
    }

    G.restore();
    return make_pair(ordering, right_degrees);
}

/** Makes a copy of the current state of the neighbours */
const vector<Vertex*> get_neighbours(const Graph& G, const Vertex* v)
{
    vector<Vertex*> neighbours = vector<Vertex*>(v->neighbors.size());
    for(int i = 0; i < v->neighbors.size(); ++i)
    {
        neighbours[i] = v->neighbors[i];
    }
    return neighbours;
}

pair<unique_ptr<vector<Vertex*>>, unique_ptr<vector<vector<Vertex*>>>> degeneracy_ordering_rN(Graph& G)
{
    G.set_restore();

    unique_ptr<vector<Vertex*>> ordering = make_unique<vector<Vertex*>>();
    unique_ptr<vector<vector<Vertex*>>> right_neighbourhoods = make_unique<vector<vector<Vertex*>>>(G.V);

    while(G.max_degree > 0)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree][0];
        ordering->push_back(lowest_degree_vertex);

        //in our bucket graph, the neighbours of the current min vertex are also the right-neighbourhood in the
        (right_neighbourhoods.get())->at(lowest_degree_vertex->id) = get_neighbours(G, lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
    }

    G.restore();
    return make_pair(ordering, right_neighbourhoods);
}




int degeneracy(vector<Vertex*>& degeneracy_ordering, vector<int>& right_degrees)
{
    int degeneracy = 0;
    for(int i = 0; i < degeneracy_ordering.size(); ++i)
    {
        int right_degree = right_degrees[degeneracy_ordering[i]->id];
        if(right_degree > degeneracy)
        {
            degeneracy = right_degree;
        }
    }
    return degeneracy;
}

int degeneracy(vector<Vertex*>& degeneracy_ordering, vector<vector<Vertex*>>& right_neighbourhoods)
{
    int degeneracy = 0;
    for(int i = 0; i < degeneracy_ordering.size(); ++i)
    {
        int right_degree = right_neighbourhoods[degeneracy_ordering[i]->id].size();
        if(right_degree > degeneracy)
        {
            degeneracy = right_degree;
        }
    }
    return degeneracy;
}

//-------------------------- GARBAGE COLLECTION ---------------------------------------

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