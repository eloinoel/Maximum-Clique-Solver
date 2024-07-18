#pragma once


#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>

class Graph;
class Vertex;

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

class KTruss
{
//variables
public:
    /* map: edge (u, v) --> edge_id */
    std::unordered_map<std::pair<unsigned int, unsigned int>, int, undirected_pair_hash> edge_map;
    /* how many triangles each edge is a part of */
    std::vector<int> edge_support;

private:

//functions
public:
    /** init data structures */
    KTruss(Graph& G, std::vector<Vertex*>& degeneracy_ordering);

    /**
     * Elias TODO: forward algorithm in https://publikationen.bibliothek.kit.edu/1000007159/4541 p. 32 
     */ 
    std::vector<Triangle> compute_triangles(Graph& G);

private:
};