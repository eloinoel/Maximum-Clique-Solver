/**
 * @brief
 * Maximum Clique Solver using ideas from 'Why is maximum clique often easy in practice',
 * J. Walteros and A. Buchanan https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/
#pragma once

#include <vector>

class Graph;
class Vertex;

class SolverViaVC
{
//-----------------------Variables-----------------------
public:

    Graph* G;

    std::vector<Vertex*> degeneracy_ordering;
   std::vector<std::vector<Vertex*>> right_neighbourhoods;
    int d;

    int cliqueUB;
    int cliqueLB;

//-----------------------Functions-----------------------
public:

    /**
     * Determines maximum clique size in the graph. Iterates function `solve_via_vc_for_p()` for
     * increasing values of p, thus increasing the clique core gap.
     * @details Constructs complement graphs on "small" subgraphs of G and uses a minimum
     * vertex cover solver. Uses the observation that MC(G) = N - VC(¬G).
     * @returns maximum clique size in the graph G
     */
    int solve_via_vc(Graph& G);

    /**
     * @brief Determines whether G has a clique of size (d + 1) - p. In other words,
     * it determines whether the clique-core gap is at most p.
     */
    bool solve_via_vc_for_p(Graph& G, unsigned int p);
};
