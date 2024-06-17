#include "branch_and_bound.h"

/**
 * @brief Get candidate set P
 * 
 * @param G out current Graph
 * @return candidate set P
 */
vector<Vertex*> get_candidates(Graph& G){
    
    std::vector<Vertex*> candidates;
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        for(Vertex* u: candidates_of_same_degree){
            candidates.push_back(u);
        }
    }

    return candidates;
}

/**
 * @brief Basic branch and bound framework that can be extended with bounds, reductions and orderings.
 * 
 * The current graph has the candidate set P and our previously constructed local clique C implicitly stored
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

    //get P
    vector<Vertex*> candidates = get_candidates(G);

    if(bounding1(G, candidates, maximum_clique)==BOUNDING::CUTOFF){
        G.restore();
        return;
    }

    //for all v ∈ P
    while(!candidates.empty()){
        Vertex* branch_vertex = candidates.back();
        candidates.pop_back();

        G.set_restore();

        //C' = C ∪ {v} 
        G.MM_clique_add_vertex(branch_vertex);
        
        //P' = P ∩ N(v)
        G.MM_induced_subgraph(branch_vertex->neighbors);

        //recursive call of BnB(P', C', C^*)
        branch_and_bound(G, maximum_clique);

        G.restore();

        //P = P\{v}
        G.MM_discard_vertex(branch_vertex);
 
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
