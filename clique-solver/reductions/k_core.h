/**
 * Used in 'Why is maximum clique often easy in practice', J. Walteros and A. Buchanan
 * https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/

#pragma once
#include "graph.h"


/**
 * @brief Discard vertices which can't be in the maximum clique.
 * Given a lower bound L on the maximum clique vertex number,
 * the 'L-1'-core can be applied to discard vertices that have less than L-1 neighbours.
 * @note O(V + E)
 */
inline void apply_k_core(Graph &G, vector<Vertex*> maximum_clique){
    int to_low_degree = maximum_clique.size()-G.partial.size()-2;
    for(int i = min(to_low_degree, (int)G.deg_lists.size()-1); i>=0 ;i--){
        while(!G.deg_lists[i].empty()){
            G.MM_discard_vertex(G.deg_lists[i].back());
        }
    }
}

