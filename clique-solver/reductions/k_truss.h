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

class Graph;
class Vertex;

constexpr int INF = std::numeric_limits<int>::infinity();

// Custom hash function for undirected pairs, mapping edges (u, v) and (v, u) to the same id
struct undirected_pair_hash {
    std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const {
        // Ensure that (u, v) and (v, u) hash to the same value
        std::size_t seed = 0;
        seed ^= std::hash<unsigned int>{}(p.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<unsigned int>{}(p.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

/* a triangle contains three vertex indices */
using Triangle = unsigned int[3];

typedef struct _edge {
    int id;
    int support;
} edge;

class KTruss
{
//variables
public:
    /* map: edge (u, v) --> edge_id */
    std::unordered_map<std::pair<unsigned int, unsigned int>, int, undirected_pair_hash> edge_map;
    /* how many triangles each edge is a part of */
    std::vector<edge*> edge_support;
    /* sorted edges in ascending order of their support */
    std::vector<edge*> sorted_edges;
    /* shallow copy in case needed */
    std::shared_ptr<Graph> H = nullptr;

    /* k-class: edges of trussness k stored as edge ids*/
    std::vector<std::vector<int>> k_classes;
    int max_k = 0;

private:

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
     * @brief remove all edges in graph whose endpoints have fewer than L - 2 common neighbours (triangles)
     */
    void reduce(int lower_clique_bound);

    /**
     * @brief compute community degeneracy c. UB = c + 2
     */
    size_t upper_bound();

private:
    /**
     * Elias TODO: forward algorithm in https://publikationen.bibliothek.kit.edu/1000007159/4541 p. 32 
     */ 
    std::vector<Triangle> compute_triangles(Graph& G, std::vector<Vertex*>& degeneracy_ordering);

    /**
     * @note O(E)
     */
    void init_edge_map(Graph& G);

    /**
     * @note O(m^1.5)
     */
    void compute_support(std::vector<Triangle>& triangles, const Graph& G);
};

#pragma once



/** 
 * Bucketsort for edges
 * @note
 */
class Buckets
{
public:
    /** store the neighbourhood indeces in buckets --> sorted */
    std::vector<std::vector<edge*>> buckets;
private:
    size_t _size = 0;

public:
    //default constructor
    Buckets() = default;

    /**
     * put the first N - d right neighbourhoods in buckets (sort)
     */
    Buckets(std::vector<edge*>& edge_support);
    void insert(edge* e);

    std::vector<edge*> get_sorted_edges();

    void print();

    inline size_t size() { return _size; }
};
