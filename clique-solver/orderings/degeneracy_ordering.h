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
 * @returns right_degrees of vertices in the ordering, other vertices have value -1
 */
std::pair<std::vector<Vertex*>,std::vector<int>> degeneracy_ordering(Graph& G);
std::pair<std::vector<Vertex*>, std::vector<std::vector<Vertex*>>> degeneracy_ordering_rN(Graph& G);
/**
 * The degeneracy of a graph can be computed from the degeneracy ordering.
 * First execute function `degeneracy_ordering(Graph& G)` and input return values into this fn
 * @note O(V)
 * @returns degeneracy of the current graph
 */
int degeneracy(std::vector<Vertex*>& degeneracy_ordering, std::vector<int>& right_degrees);
int degeneracy(std::vector<Vertex*>& degeneracy_ordering, std::vector<std::vector<Vertex*>>& right_neighbourhoods);

/**
 * @returns degeneracy of the current graph
 * @note iteratively applies k-core
 */
int degeneracy(Graph& G);