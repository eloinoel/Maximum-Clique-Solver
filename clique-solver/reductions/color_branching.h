#pragma once
#include "graph.h"

/**
 * @brief reducing branching verticies B by coloring
 * 
 * @param partial_clique C
 * @param branching_candidates B 
 * @param maximum_clique C*
 * @return new B
 */
vector<Vertex*> reduce_B_by_coloring(vector<Vertex*> partial_clique, vector<Vertex*> branching_candidates, vector<Vertex *> maximum_clique);