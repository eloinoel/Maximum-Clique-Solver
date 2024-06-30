#include <vector>
#include <unordered_map>
#include <deque>

class Graph;
class Vertex;

/**
 * @brief incremental MaxSAT reasoning to reduce the set of branching vertices B
 * 
 * @param ordering b_1, ..., b_k, a_1, ... a_n
 * @param coloring c: V -> N
 * @param r current max clique size + 1
 * @return reduced branching set b_i, ..., b_j
 */
std::vector<Vertex*> inc_max_sat(std::vector<Vertex*>& ordering, std::unordered_map<Vertex*, int> &coloring, int r);