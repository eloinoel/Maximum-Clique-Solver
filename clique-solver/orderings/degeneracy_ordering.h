/**
 * Used in 'Why is maximum clique often easy in practice', J. Walteros and A. Buchanan
 * https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/

#pragma once

#include <vector>
#include <utility> //contains std::pair

class Graph;
class Vertex;

/**
 * @brief Computes degeneracy vertex ordering and degeneracy of valid vertices in the given graph.
 * A degeneracy ordering is also a minimum degree ordering.
 * @note O(E + V), source: https://dl.acm.org/doi/pdf/10.1145/2402.322385
 * @returns minimum degree/degeneracy ordering,
 * @returns degeneracy of the current graph
 */
std::pair<std::vector<Vertex*>, int> degeneracy_ordering(Graph& G);
