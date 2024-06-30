#include "graph.h"

using namespace std;

void apply_k_core(Graph &G, vector<Vertex*>& maximum_clique) {
    int to_low_degree = maximum_clique.size() - G.partial.size() - 2;
    for(int i = min(to_low_degree, (int)G.deg_lists.size()-1); i>=0 ;i--){
        while(!G.deg_lists[i].empty()){
            G.MM_discard_vertex(G.deg_lists[i].back());
        }
    }
}

void apply_k_core(Graph &G, int lowerCliqueBound)
{
    while((int) G.min_degree < lowerCliqueBound - 1)
    {
        Vertex* lowest_degree_vertex = G.deg_lists[G.min_degree].front();
        G.MM_discard_vertex(lowest_degree_vertex);
    }
}

/**
 * @brief check if G is empty
 *
 * @param G
 * @return true if G is empty
 * @return false if G is not empty
 */
bool empty_graph(Graph& G){
    for(vector<Vertex*> candidates_of_same_degree : G.deg_lists){
        for(Vertex* u: candidates_of_same_degree){
            return false;
        }
    }
    return true;
}

/**
 * @brief calculates the maximum k-core iteratively increasing from 0-core
 *
 * @param G
 * @return k of maximum k core
 */
unsigned long max_k_core(Graph& G){
    G.set_restore();
    unsigned long i;
    for(i=0;!(empty_graph(G));i++){
        apply_k_core(G, i+1);
    }
    G.restore();
    return i-2;
}