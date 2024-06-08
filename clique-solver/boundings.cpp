#include "../vc-solver/graph.h"
#include "boundings.h"

/**
 * @brief if P + C <= C^* then CutOff
 * 
 * @return true |P| + C| <= |C^*|
 * @return false |P| + |C| > |C^*|
 */
Bounding_status bounding1(Graph& G, vector<Vertex*>& maximum_clique){
    int candidate_size = 0;
    for(int i=0; i<=G.max_degree; i++){
        candidate_size+=G.deg_lists[i].size();
    }

    if(candidate_size + G.sol_size + 2 <  maximum_clique.size()){
        return Bounding_status::CUTOFF;
    }
    return Bounding_status::NO_CUTOFF;
}
