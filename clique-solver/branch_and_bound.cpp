#include "../vc-solver/graph.h"

/**
 * @brief Get candidate set P
 * 
 * @param G out current Graph
 * @return candidate set P
 */
vector<Vertex*> get_candidates(Graph& G){
    
    std::vector<Vertex*> candidates;
    for(vector<Vertex*> list: G.deg_lists){
        for(Vertex* u: list){
            candidates.push_back(u);
        }
    }

    return candidates;
}

/**
 * @brief Basic branch and bound framework that can be extended with bounds, reductions and orderings.
 * 
 * The current graph has the candidate set P and our previously constructed local clique C implicitly stored with
 * P = {v in G.V | with v.status = UNKNOWN} and C = {v in G.V | with v.status = INCLUDE}
 * 
 * 
 * @param G our current graph
 * @param maximum_clique so far (C^*) 
 */
void branch_and_bound(Graph& G, vector<Vertex*>& maximum_clique){ //BnB(P, C, C^*)
    G.set_restore();

    //if C > C^* then
    if(G.partial.size()>maximum_clique.size()){
        //C^* = C
        maximum_clique= G.partial;
    }

    //for all v ∈ P
    vector<Vertex*> candidates = get_candidates(G);
    while(!candidates.empty()){
        Vertex* branch_vertex = candidates.back();

        G.set_restore();

        //P' = P ∩ (N(v) ∪ {v})
        G.MM_induced_neighborhood(branch_vertex);

        //C' = C ∪ {v} & P' = P'\{v}
        G.MM_clique_add_vertex(branch_vertex);

        //recursive call of BnB(P', C', C^*)
        branch_and_bound(G, maximum_clique);

        G.restore();

        //P = P\{v}
        candidates.pop_back();
        G.MM_induced_subgraph(candidates);
 
    }

    G.restore();
}
/**
 * @brief Basic branch and bound framework for maximum clique
 * 
 * @param G
 * @return maximum clique 
 */
vector<Vertex*> branch_and_bound(Graph& G){
    std::vector<Vertex*> maximum_clique;
    branch_and_bound(G,maximum_clique);
    return maximum_clique;
}
