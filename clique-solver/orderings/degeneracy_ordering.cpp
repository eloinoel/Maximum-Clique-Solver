#include "degeneracy_ordering.h"
#include "graph.h"
#include "k_core.h"
#include "colors.h"

using namespace std;

pair<vector<Vertex*>, vector<int>> degeneracy_ordering(Graph& G)
{
    G.set_restore();

    vector<Vertex*> ordering = vector<Vertex*>(G.V.size());
    vector<int> right_degrees = vector<int>(G.V.size(), -1); //init with -1

    int N = G.N;
    for(int i = 0; i < N; ++i)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree].front();
        assert(i < (int) ordering.size());
        ordering[i] = lowest_degree_vertex;

        //in our bucket graph, the degree of the current min vertex is also the rightDegree in the ordering
        right_degrees[lowest_degree_vertex->id] = deg(lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
    }

    G.restore();
    return make_pair(ordering, right_degrees);
}

/** Makes a copy of the current state of the neighbours */
const vector<Vertex*> get_neighbours(const Vertex* v)
{
    vector<Vertex*> neighbours = vector<Vertex*>(v->neighbors.size());
    for(int i = 0; i < (int) v->neighbors.size(); ++i)
    {
        neighbours[i] = v->neighbors[i];
    }
    return neighbours;
}

pair<vector<Vertex*>, vector<vector<Vertex*>>> degeneracy_ordering_rN(Graph& G)
{
    G.set_restore();

    vector<Vertex*> ordering = vector<Vertex*>(G.V.size());
    vector<vector<Vertex*>> right_neighbourhoods = vector<vector<Vertex*>>(G.V.size());

    int N = G.N;
    for(int i = 0; i < N; ++i)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree].front();
        assert(i < (int) ordering.size());
        ordering[i] = lowest_degree_vertex;

        //in our bucket graph, the degree of the current min vertex is also the rightDegree in the ordering
        right_neighbourhoods[lowest_degree_vertex->id] = get_neighbours(lowest_degree_vertex);
        G.MM_discard_vertex(lowest_degree_vertex);
    }

    G.restore();
    return make_pair(ordering, right_neighbourhoods);
}




int degeneracy(vector<Vertex*>& degeneracy_ordering, vector<int>& right_degrees)
{
    int degeneracy = 0;
    for(int i = 0; i < (int) degeneracy_ordering.size(); ++i)
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
    for(int i = 0; i < (int) degeneracy_ordering.size(); ++i)
    {
        int right_degree = right_neighbourhoods[degeneracy_ordering[i]->id].size();
        if(right_degree > degeneracy)
        {
            degeneracy = right_degree;
        }
    }
    return degeneracy;
}

int degeneracy(Graph& G)
{
    return max_k_core(G);
}

void print_degeracy_ordering_and_rneighbourhoods(vector<Vertex*>& degeneracy_ordering, vector<vector<Vertex*>>& right_neighbourhoods, Graph& G)
{
    cout << RED << "------------ Degeneracy ordering ------------" << RESET << endl;
    int i = 0;
    for(Vertex* v : degeneracy_ordering)
    {
        cout << CYAN << i << GREEN << ", v " << G.name_table[v->id] << ": " << RESET;
        for(Vertex* n : right_neighbourhoods[v->id])
        {
            cout << G.name_table[n->id] << ", ";
        }
        cout << endl;
        i++;
    }
    cout << RED << "------------ END degeneracy ordering ------------" << RESET << endl;
}