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

typedef struct rn_{
    std::vector<Vertex*> neigh;
    int vc_size = 0;
    std::vector<std::string> sol;
}rn;

class SolverViaVC
{
//-----------------------Variables-----------------------
public:

    std::vector<Vertex*> degeneracy_ordering;
    std::vector<rn> right_neighbourhoods;
    int d;

    int clique_UB;
    int clique_LB;
    std::vector<Vertex*> LB_clique_vertices;

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
    bool solve_via_vc_for_p(Graph& G, size_t p, int LB);

    bool solve_via_vc_for_p_with_sorting(Graph& G, size_t p, int LB);

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

    Graph get_complement_subgraph(Graph & G, std::vector<Vertex*>& induced_set, int LB);

    /**
     * @param complementGraph graph where vertex cover was solved for
     * @param ordering_vertex vertex which induced the complementGraph with its right-neighbourhood
     * @note stores solution in this.maximum_clique, O(complement.V * vc_size)
     */
    void extract_maximum_clique_solution(Graph& complementGraph, Vertex* o = nullptr);
    std::vector<std::string> extract_maximum_clique_solution_from_rn(rn& right, Graph& complementGraph);

    Vertex* get_vertex_by_name(Graph& G, std::string name);

    /**
     * @brief Sets @maximum_clique to the vertices in the input vector
     * 
     */
    void set_maximum_clique(std::vector<Vertex*>& vertices, Graph& G);

};


/** 
 * Bucketsort for right neighbourhoods, will only work for positive elements
 * @note init: O(n-d), insert: O(1), get_next: O(1), at most O(n) cumulative space+time in main algorithm
 */
class Buckets
{
public:
    /** store the neighbourhood indeces in buckets --> sorted */
    std::vector<std::vector<int>> buckets;

private:
    int current_bucket = -1;
    int current_index = -1;

    int start_bucket = -1;
    int start_index = -1;

public:
    static Buckets init(std::vector<Vertex*>& degeneracy_ordering, std::vector<rn>& right_neighbourhoods, int d);
    void insert(int neighbourhood_index, int neighbourhood_size);

    /** 
     * @returns neighbourhood index <= the current index, -1 if no more elements
     */
    int get_next();

    /**
     * iterator end value to test against in loops
     */
    inline int end() { return -1; };

    /** 
     * reset iterator 
     */
    inline void reset() { current_bucket = start_bucket; current_index = start_index; };
};
