#include "lower_bounds.h"
#include "graph.h"

int degeneracy_ordering_LB(std::vector<Vertex*>& degeneracy_ordering, std::vector<int>& right_degrees)
{
    int lower_bound = 0;
    int n = degeneracy_ordering.size();
    for(int i = 0; i < n; ++i)
    {
        int num_right_vertices = n - i - 1;
        int cur_right_degree = right_degrees[degeneracy_ordering[i]->id];
        if(cur_right_degree == num_right_vertices) //found a clique
        {
            return n - i; //{vertex} ∪ rN(vertex) => clique
        }
    }
    return lower_bound;
}