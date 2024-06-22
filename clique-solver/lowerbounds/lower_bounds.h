#pragma once

#include <vector>

class Vertex;

/**
 * A simple lower bound computed from the degeneracy ordering.
 * First execute function `degeneracy_ordering(Graph& G)` and input return values into this fn
 * @note O(V), source: https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
 * @returns degeneracy of the current graph
 */
int degeneracy_ordering_LB(const std::vector<Vertex*>& degeneracy_ordering, const std::vector<int>& right_degrees);
std::vector<Vertex*> degeneracy_ordering_LB(const std::vector<Vertex*>& degeneracy_ordering, const std::vector<std::vector<Vertex*>>& right_neighbourhoods);
