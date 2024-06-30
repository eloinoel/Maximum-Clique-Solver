#pragma once

#include <vector>
#include <unordered_map>
class Vertex;
class Graph;
class Color_container;

/**
 * @brief reducing branching verticies B by coloring
 *
 * @param partial_clique C
 * @param branching_candidates B
 * @param maximum_clique C*
 * @return new B
 */
std::vector<Vertex*> reduce_B_by_coloring(std::vector<Vertex*>& partial_clique, std::vector<Vertex*>& branching_candidates, std::vector<Vertex *>& maximum_clique);

/**
 * @brief greedy coloring
 * @note api is optimized for the case that IncMaxSat follows
 * 
 * @param G 
 * @param r 
 * @param global_ordering 
 * @return Color_container 
 */
Color_container dsatur(Graph& G, int r, std::vector<Vertex*>& global_ordering);

struct Color_container{
    std::unordered_map<Vertex*, int> coloring;
    std::vector<Vertex*> new_ordering;
    std::vector<Vertex*> B;
    std::vector<std::vector<Vertex*>> A;
    

    Color_container(std::unordered_map<Vertex*, int> c, std::vector<Vertex*> o, std::vector<Vertex*> b, std::vector<std::vector<Vertex*>> a):coloring(c),B(b),A(a),new_ordering(o){};
};