#pragma once

#include <vector>

class Vertex;
class Graph;

/**
 * @param degeneracy obtain this value with the function `degeneracy_ordering()`
 * @returns simple upper bound on the maximum clique size
 * @note source: https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
 */
inline int degeneracy_UB(int degeneracy)
{
    return degeneracy + 1;
}

/**
 * @param community_degeneracy largest value c for which there exists a subgraph in which
 * the endpoints of each edge have at least c neighbors in common.
 * @returns simple upper bound on the maximum clique size
 * @note source: https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
 */
//CANNOT BE USED WITHOUT K-TRUSS
// inline int community_degeneracy_UB(int community_degeneracy)
// {
//     return community_degeneracy + 2;
// }


enum BOUNDING{
    CUTOFF,
    NO_CUTOFF
};

/**
 * @brief if |C|+|P|<=|C*|
 *
 * @param G contains C
 * @param candidates is P
 * @param maximum_clique is C*
 * @return CUTOFF or NO_CUTOFF
 */
BOUNDING upper_bound(Graph& G, std::vector<Vertex*>& maximum_clique);