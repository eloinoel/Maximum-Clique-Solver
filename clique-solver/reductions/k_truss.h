#pragma once

/**
 * Sources:
 * compute triangles: https://publikationen.bibliothek.kit.edu/1000007159/4541 p. 32
 * k-truss algorithm: http://vldb.org/pvldb/vol5/p812_jiawang_vldb2012.pdf
 * data reduction, community degeneracy, upper bound: https://optimization-online.org/wp-content/uploads/2018/07/6710.pdf
 */

#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>
#include <limits>
#include <memory>
#include <cassert>
#include <array>

class Graph;
class Vertex;
class Edge;

constexpr int INF = std::numeric_limits<int>::infinity();

// Custom hash function for undirected pairs, mapping edges (u, v) and (v, u) to the same id
struct undirected_pair_hash {
    std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const {
        unsigned int first = std::min(p.first, p.second);
        unsigned int second = std::max(p.first, p.second);

        std::size_t seed = 0;
        seed ^= std::hash<unsigned int>{}(first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<unsigned int>{}(second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    }
};

struct undirected_pair_equal {
    bool operator()(const std::pair<unsigned int, unsigned int>& lhs,
                    const std::pair<unsigned int, unsigned int>& rhs) const {
        return (lhs.first == rhs.first && lhs.second == rhs.second) ||
               (lhs.first == rhs.second && lhs.second == rhs.first);
    }
};

/* a triangle contains three vertex indices */
using Triangle = std::array<unsigned int, 3>;

typedef struct _edge {
    Edge* graph_edge;
    int id; //this is used for all data structures in KTruss
    int support;
    int sorted_edges_index;
} edge;

enum class PrintVertices
{
    Names,
    IDs,
    None
};

class KTruss
{
//variables
public:
    /* map: edge (u, v) --> edge_id, map will be empty after calling `compute_k_classes`*/
    std::unordered_map<std::pair<unsigned int, unsigned int>, int, undirected_pair_hash, undirected_pair_equal> edge_map;
    /* how many triangles each edge is a part of */
    std::vector<edge*> edge_support;
    /* sorted edges in ascending order of their support */
    std::vector<edge*> sorted_edges;
    /* for each support contains the index of the first element of the support region in `sorted_edges`*/
    std::vector<int> support_region_index;
    /* shallow copy in case needed */
    std::shared_ptr<Graph> H = nullptr;

    /* k-class: edges of trussness k stored as edge ids*/
    std::vector<std::vector<Edge*>> k_classes;

private:
    #if !NDEBUG
    std::unordered_map<int, bool> removed_edges;
    #endif
    bool computed_k_classes = false;

//functions
public:
    /** 
     * @brief init data structures
     * @note O(m^1.5)
     */
    KTruss(const Graph& G, std::vector<Vertex*>& degeneracy_ordering);

    /**
     * @brief need to free edge pointers
     */
    ~KTruss();

    /**
     * @brief compute k-classes, which contain all edges of trussness k
     * where k is the largest number for which the edge is contained in a k-truss.
     * @note Algorithm 2 in http://vldb.org/pvldb/vol5/p812_jiawang_vldb2012.pdf
     * @returns results are written to variable `k_classes`
     */
    void compute_k_classes();

    /**
     * @brief remove all edges in graph whose endpoints have fewer than L - 2 common neighbours (triangles)
     * @param G graph to reduce, needs to be same as the one used in constructor
     * @note can be called before executing `compute_k_classes` or after
     * @returns number of removed edges
     */
    size_t reduce(Graph& G, int lower_clique_bound);

    /**
     * @brief compute community degeneracy c. UB = c + 2
     * @note requires `compute_k_classes` to be called first	
     */
    int upper_bound();

    void print_edge(edge* e, PrintVertices print_vertices = PrintVertices::IDs);

    void print_triangles(std::vector<Triangle>& triangles);

    void print_support(PrintVertices print_vertices = PrintVertices::IDs);

    void print_k_classes(PrintVertices print_vertices = PrintVertices::IDs);

private:
    /**
     * Elias TODO: forward algorithm in https://publikationen.bibliothek.kit.edu/1000007159/4541 p. 32 
     */ 
    std::vector<Triangle> compute_triangles(std::vector<Vertex*>& degeneracy_ordering);

    /**
     * @note O(E)
     */
    void init_edge_map(Graph& G);

    /**
     * @note O(m^1.5)
     */
    void compute_support(std::vector<Triangle>& triangles, const Graph& G);

    inline bool is_edge(unsigned int u, unsigned int v)
    {
        return edge_map.find(std::make_pair(u, v)) != edge_map.end();
    };

    edge* get_edge(unsigned int u, unsigned int v);

    /**
     * @brief support(edge*)--
     */
    void decrement_support(edge* e);

    /**
     * @brief update the position of edge e in sorted_edges one support to the left
     * @param current_iteration_index edge that is currently being looked at in `compute_k_classes`
     * @returns new_index where new_index > current_iteration_index
     */
    int reorder_edge(edge* e, int current_iteration_index);

    void swap_edge(edge*e, int swap_index);

    void remove_edge_from_graph(edge* e);

    inline int support(edge* e)
    {
        assert(e != nullptr);
        return e->support;
    }
};

#pragma once



/** 
 * Bucketsort for edges
 * @note
 */
class BucketSort
{
public:
    /** store the neighbourhood indeces in buckets --> sorted */
    std::vector<std::vector<edge*>> buckets;
private:
    size_t _size = 0;

public:
    //default constructor
    BucketSort() = default;

    /**
     * put the first N - d right neighbourhoods in buckets (sort)
     */
    BucketSort(std::vector<edge*>& edge_support);
    void insert(edge* e);

    std::vector<edge*> get_sorted_edges();

    void print();

    inline size_t size() { return _size; }
};
