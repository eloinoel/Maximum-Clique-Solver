#include "../vc-solver/graph.h"

/**
 * FIXME:
 * Input: bash compile.sh --release && ./build/release/mc_solver < benchmark/data/014_random_80_2.dimacs
 * Output: segmentation fault  ./build/release/mc_solver < benchmark/data/014_random_80_2.dimacs
/

/**
 * @brief Basic branch and bound framework that can be extended with bounds, reductions and orderings.
 * 
 * The current graph has the candidate set P and our previously constructed local clique C stored with
 * P = G.deg_lists and C = G.partial
 * 
 * 
 * @param G our current graph
 * @param maximum_clique so far (C^*) 
 */
void branch_and_bound(Graph& G, vector<Vertex*>& maximum_clique){ //BnB(P, C, C^*)
    G.set_restore();

    //if C > C^* then
    if(G.sol_size>maximum_clique.size()){
        //C^* = C
        maximum_clique.clear();
        for(Vertex* v: G.partial)maximum_clique.push_back(v);
    }

    //filter P from G
    std::vector<Vertex*> candidates;
    for(vector<Vertex*> candidates_of_same_degree: G.deg_lists){
        for(Vertex* candidate: candidates_of_same_degree){
            candidates.push_back(candidate);
        }
    }

    //for all v ∈ P
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
