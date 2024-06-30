#include <vector>

class Vertex;
class Graph;


/**
 * @brief is a max-clique algorithm designed for large, sparse graphs. He uses a preprossesing
 * procedure that is intended to reduce the sparse graph to a small dense graph and then uses 
 * coloring + IncMaxSat in the well-known branch and bound process
 * 
 * @param G 
 * @return maximum clique of G 
 */
std::vector<Vertex*> lmc(Graph& G);