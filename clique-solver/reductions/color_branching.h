#pragma once

#include <vector>

class Vertex;

/**
 * @brief reducing branching verticies B by coloring
 *
 * @param partial_clique C
 * @param branching_candidates B
 * @param maximum_clique C*
 * @return new B
 */
std::vector<Vertex*> reduce_B_by_coloring(std::vector<Vertex*>& partial_clique, std::vector<Vertex*>& branching_candidates, std::vector<Vertex *>& maximum_clique);