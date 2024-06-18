#pragma once

#include <vector>

class Graph;
class Vertex;

/**
 * @brief recursive branch and bound algorithm
 */
void branch_and_bound(Graph& G, std::vector<Vertex*>& maximum_clique);

/**
 * @brief Branch and bound wrapper, call this from externally
 */
std::vector<Vertex*> branch_and_bound_mc(Graph& G);