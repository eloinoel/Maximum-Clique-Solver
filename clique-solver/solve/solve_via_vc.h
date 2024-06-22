/**
 * @brief
 * Maximum Clique Solver using ideas from 'Why is maximum clique often easy in practice',
 * J. Walteros and A. Buchanan https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/
#pragma once

#include <vector>
#include <cstddef>
#include <string>

class Graph;
class Vertex;

class SolverViaVC
{
//-----------------------Variables-----------------------
public:

    std::vector<Vertex*> degeneracy_ordering;
    std::vector<std::vector<Vertex*>> right_neighbourhoods;
    int d;

    int clique_UB;
    int clique_LB;

    /** 
     * this will be filled after solve_via_vc() is called 
     * */
    std::vector<std::string> maximum_clique;

private: 
    //Graph solution_complement_graph;
    

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

    bool is_clique(std::vector<Vertex*> vertices);
    std::vector<Vertex*> convert_vertex_list(Graph& G, std::vector<std::string> names);

private:
    /**
     * @brief Determines whether G has a clique of size (d + 1) - p. In other words,
     * it determines whether the clique-core gap is at most p.
     * @note call `solve_via_vc()` instead of this function
     */
    bool solve_via_vc_for_p(Graph& G, size_t p);

    /**
     * @param p max assumed possible clique-core gap in current iteration
     * @returns Returns a candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
     */
    std::vector<Vertex*> get_candidate_set(size_t p);

    /**
     * @param p max assumed possible clique-core gap in current iteration
     * @returns Returns a remaining set R = {vi ∈ V | i > n − d}
     */
    std::vector<Vertex*> get_remaining_set();

    /**
     * @param complementGraph graph where vertex cover was solved for
     * @param ordering_vertex vertex which induced the complementGraph with its right-neighbourhood
     * @note stores solution in this.maximum_clique, O(complement.V * vc_size)
     */
    void extract_maximum_clique_solution(Graph& complementGraph, Vertex* o = nullptr);

    Vertex* get_vertex_by_name(Graph& G, std::string name);
};
