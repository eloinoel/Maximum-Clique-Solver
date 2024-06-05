#include "../vc-solver/graph.h"

/**
 * @brief returns the candidate set P
 * 
 * @param G out current Graph
 * @return our candidates P
 */
vector<Vertex*> get_candidates(Graph& G){
    std::vector<Vertex*> candidates;
    
    //we go through the degree list in descending order to build the candidate list in ascending order
    for(int i=G.max_degree; i>=1; i--){
        for(Vertex* u: G.deg_lists[i]){
            candidates.push_back(u);
        }
    }

    return candidates;
}

/**
 * @brief Get one candidate v ∈ P
 * 
 * @param G out current Graph
 * @return candidate v ∈ P
 */
Vertex* get_candidate(Graph& G){
    
    //we return the first vertex we find in the degree list
    for(int i=1; i<=G.max_degree; i++){
        for(Vertex* u: G.deg_lists[i]){
            return u;
        }
    }

    return nullptr;
}

/**
 * @brief We reduce P to P ∩ (N(v) ∪ {v})
 * 
 * @param G our current graph
 * @param v our current branching vertex
 */
void MM_discard_strangers(Graph& G, Vertex* v){

    vector<Vertex*> candidates = get_candidates(G);

    //if there is a stranger u in P where u is not in N(v) and u != v we EXCLUDE him and continue search P\{u}
    for(Vertex* u : candidates){
        if(u!=v && !v->adjacent_to(u)){

            //EXCLUDE u
            G.MM_discard_vertex(u);

            //EXCLUDE strangers im remaining Graph
            MM_discard_strangers(G,v);
            break;
        }
    }
}

/**
 * @brief Basic Branch and Bound Framework that can be extended with bounds, reductions and orderings
 * 
 * The current graph has the candidate set P and our previously constructed local clique C implicitly stored with
 * P = {v in G.V | with v.status = UNKNOWN} and C = {v in G.V | with v.status = INCLUDE}
 * 
 * 
 * @param G our current graph
 * @return maximum clique size
 */
int branch_and_bound(Graph& G){
    G.set_restore();

    //we ignore all verticies v with |N(v)|==0 but therefor use G.sol_size +1 instead of G.sol_size
    int maximum_clique_size = G.sol_size+1;

    //for all v ∈ P
    for(Vertex* branch_vertex = get_candidate(G); branch_vertex!=nullptr; branch_vertex = get_candidate(G)){
        G.set_restore();

        //P' = P ∩ N(v)
        MM_discard_strangers(G, branch_vertex);
        //C' = C ∪ {v}
        G.MM_select_vertex(branch_vertex);

        //recursive call of BnB(P',C')
        maximum_clique_size = max(maximum_clique_size, branch_and_bound(G));
        
        G.restore();

        //P = P\{v}
        G.MM_discard_vertex(branch_vertex);

    }

    G.restore();
    return maximum_clique_size;
}


