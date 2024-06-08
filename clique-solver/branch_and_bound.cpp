#include "../vc-solver/graph.h"
#include "boundings.h"

/**
 * @brief Get candidate set P
 * 
 * @param G out current Graph
 * @return P
 */
vector<Vertex*> get_candidates(Graph& G){
    
    std::vector<Vertex*> candidates;
    
    //get the candidate set P stored implicitly in G
    for(int i=G.max_degree; i>=1; i--){
        for(Vertex* u: G.deg_lists[i]){
            candidates.push_back(u);
        }
    }
    return candidates;
}

/**
 * @brief We reduce P to P ∩ (N(v) ∪ {v})
 * 
 * @param G our current graph
 * @param v our current branching vertex
 */
void MM_discard_strangers(Graph& G, Vertex* v){

    
    //get the candidate set P stored implicitly in G
    vector<Vertex*> candidates = get_candidates(G);

    //if there is a candidate in P that is not in N(v) and not v itself then its a stranger
    std::vector<Vertex*> strangers;
    for(Vertex* candidate : candidates){
        if(candidate!=v && !v->adjacent_to(candidate)){
            strangers.push_back(candidate);
        }
    }

    //EXCLUDE strangers
    for(Vertex* stranger: strangers){
        if(stranger->degree()>0){
            G.MM_discard_vertex(stranger);
        }
    }


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

    if(bounding1(G,maximum_clique)==Bounding_status::CUTOFF){
        G.restore();
        return;
    }

    //if C > C^* then
    if(G.sol_size+1>maximum_clique.size()){

        //C^* = C
        maximum_clique.clear();
        for(Vertex* v : G.V){
            if(v->status == INCLUDED){
                maximum_clique.push_back(v);
            }
        }
        maximum_clique.push_back(G.deg_lists[G.max_degree].back());
    }

    //for all v ∈ P
    vector<Vertex*> candidates = get_candidates(G);
    for(Vertex* branch_vertex: candidates){
        //we can ignore degree zero verticies because we use them implicitly
        if(branch_vertex->degree()==0){
            continue;
        }

        G.set_restore();

        //P' = P ∩ (N(v) ∪ {v})
        MM_discard_strangers(G, branch_vertex);

        //C' = C ∪ {v} & P' = P'\{v}
        G.MM_select_vertex(branch_vertex);

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
