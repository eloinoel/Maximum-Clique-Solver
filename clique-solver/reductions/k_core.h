/**
 * Used in 'Why is maximum clique often easy in practice', J. Walteros and A. Buchanan
 * https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/

#pragma once

#include <vector>

class Vertex;
class Graph;

/**
 * @brief Discard vertices which can't be in the maximum clique.
 * Given a lower bound L on the maximum clique vertex number,
 * the 'L-1'-core can be applied to discard vertices that have less than L-1 neighbours.
 * @note O(V + E)
 */
void apply_k_core(Graph &G, std::vector<Vertex*>& maximum_clique);
void apply_k_core(Graph &G, int lowerCliqueBound);

unsigned long max_k_core(Graph& G);
