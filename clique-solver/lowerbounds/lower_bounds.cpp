#include "lower_bounds.h"
#include "graph.h"

using namespace std;

int degeneracy_ordering_LB(const vector<Vertex*>& degeneracy_ordering, const vector<int>& right_degrees)
{
    int lower_bound = 0;
    int n = degeneracy_ordering.size();
    for(int i = 0; i < n; ++i)
    {
        int num_right_vertices = n - i - 1;
        int v_right_degree = right_degrees[degeneracy_ordering[i]->id];
        if(v_right_degree == num_right_vertices) //found a clique
        {
            return n - i; //{vertex} ∪ rN(vertex) => clique
        }
    }
    return lower_bound;
}

vector<Vertex*> degeneracy_ordering_LB(const vector<Vertex*>& degeneracy_ordering, const vector<vector<Vertex*>>& right_neighbourhoods)
{
    // find lower bound
    int n = degeneracy_ordering.size();
    int i;
    for(i = 0; i < n; ++i)
    {
        int num_right_vertices = n - i - 1;
        int v_right_degree = right_neighbourhoods[degeneracy_ordering[i]->id].size();
        if(v_right_degree == num_right_vertices) //{vertex} ∪ rN(vertex) => clique
        {
            break;
        }
    }

    // extract LB clique
    int lower_bound = n - i; //{vertex} ∪ rN(vertex) => clique
    vector<Vertex*> LBclique = vector<Vertex*>();
    LBclique.push_back(degeneracy_ordering[i]);
    for(Vertex* v : right_neighbourhoods[degeneracy_ordering[i]->id])
    {
        LBclique.push_back(v);
    }

    return LBclique;
}