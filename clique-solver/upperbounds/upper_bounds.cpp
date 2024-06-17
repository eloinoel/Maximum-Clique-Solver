#pragma once
#include "upper_bounds.h"

/**
 * @brief if |C|+|P|<=|C*|
 * 
 * @param G contains C
 * @param candidates is P
 * @param maximum_clique is C*
 * @return CUTOFF or NO_CUTOFF
 */
BOUNDING bounding1(Graph G, vector<Vertex *> candidates, vector<Vertex *> maximum_clique)
{
    if(candidates.size()+G.partial.size()<= maximum_clique.size()){
        return BOUNDING::CUTOFF;
    }
    return BOUNDING::NO_CUTOFF;
}