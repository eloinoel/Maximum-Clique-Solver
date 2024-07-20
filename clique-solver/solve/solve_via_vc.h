/**
 * @brief
 * Maximum Clique Solver using ideas from 'Why is maximum clique often easy in practice',
 * J. Walteros and A. Buchanan https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
*/
#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <limits>

#include "bucket_sort.h" // sadly necessary, make sure that no cyclic dependencies are created
#include "rn.h"

class Graph;
class Vertex;

constexpr int N_INF = -std::numeric_limits<int>::infinity();

class SolverViaVC
{
//-----------------------Variables-----------------------
public:
    bool BINARY_SEARCH = false;
    int USE_AMTS_MILLISECONDS = 0; //500-1000


    std::vector<Vertex*> degeneracy_ordering;
    /* vertex ids mapped to right neighbourhoods, right neighbourhoods store solutions*/
    std::vector<rn> right_neighbourhoods;
    /* first: VC_size, second: Vf - VC --> size will be Vf_size - VC_size */
    std::pair<int, std::vector<std::string>> remaining_set_solution = {N_INF, std::vector<std::string>()}; // avoid recomputation of vertex cover
    /* degeneracy of the Graph */
    int d;

    int clique_UB; // simple bound: d + 1
    int clique_LB = 0;
    std::vector<std::string>* LB_clique_vertices;

    /** 
     * this will be filled after solve_via_vc() is called
     * */
    std::vector<std::string> maximum_clique;

private: 
    //Graph solution_complement_graph;
    Buckets sorted_candidate_set;
    bool sorted_candidate_set_initialised = false;
    std::vector<std::string> initial_LB_solution;
    

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
     * Sort the candidate set with descending rdeg
     */
    bool solve_via_vc_for_p_with_sorting(Graph& G, size_t p);

    /**
     * linear search: solve for increasing values of p (clique core gap)
     */
    int linear_solve(Graph& G);

    /**
     * binary fashion search for clique core gap
     */
    int binary_search_solve(Graph& G);

    std::vector<Vertex*> compute_amts_LB(Graph& G);

    /**
     * @param p max assumed possible clique-core gap in current iteration
     * @returns Returns a candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
     */
    std::vector<Vertex*> get_candidate_set(size_t p);

    Buckets& get_sorted_candidate_set();

    /**
     * @param p max assumed possible clique-core gap in current iteration
     * @returns Returns a remaining set R = {vi ∈ V | i > n − d}
     */
    std::vector<Vertex*> get_remaining_set();

    Graph get_complement_subgraph(Graph & G, std::vector<Vertex*>& induced_set);

    /**
     * @param complementGraph graph where vertex cover was solved for
     * @param ordering_vertex vertex which induced the complementGraph with its right-neighbourhood
     * @note stores solution in this.maximum_clique, O(complement.V * vc_size)
     */
    std::vector<std::string> extract_maximum_clique_solution(Graph& complementGraph, Vertex* o = nullptr);
    std::vector<std::string> extract_maximum_clique_solution_from_rn(Graph& complementGraph);


    void update_LB(std::vector<std::string>& solution_candidate);

    /**
     * @brief convert vertices to string
     */
    std::vector<std::string> get_str_clique(std::vector<Vertex*>& vertices, Graph& G);

    Vertex* get_vertex_by_name(Graph& G, std::string name);
};


