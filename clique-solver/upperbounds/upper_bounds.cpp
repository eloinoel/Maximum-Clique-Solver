#pragma once
#include "upper_bounds.h"

BOUNDING upper_bound(Graph G, vector<Vertex *> maximum_clique)
{
    int candidate_set_size = 0;
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        candidate_set_size+=candidates_of_same_degree.size();
    }

    if(candidate_set_size+G.partial.size()<= maximum_clique.size()){
        return CUTOFF;
    }
    return NO_CUTOFF;
}