#include "branch_and_bound.h"
#include "upper_bounds.h"
#include "k_core.h"
#include "color_branching.h"
#include "benchmark.h"
#include "graph.h"
#include <atomic>
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
    #ifdef BENCHMARK
        if(bnb_timeout < chrono::steady_clock::now()){
            maximum_clique.clear();
            return;
        }
    #endif
    G.num_branches++;
    G.set_restore();

    //if C > C^* then
    if(G.partial.size()>maximum_clique.size()){
        //C^* = C
        maximum_clique= G.partial;
    }

    //bounding
    #ifdef BENCHMARK
        auto start = chrono::steady_clock::now();
    #endif
    BOUNDING bounding = upper_bound(G, maximum_clique);
    #ifdef BENCHMARK
        auto end = chrono::steady_clock::now();
        add_time(TECHNIQUE::BOUNDING, start, end);
    #endif 
    if(bounding == CUTOFF){
        G.restore();
        return;
    }

    //reduction of candidate set P
    #ifdef BENCHMARK
        start = chrono::steady_clock::now();
    #endif
    apply_k_core(G, maximum_clique);
    #ifdef BENCHMARK
        end = chrono::steady_clock::now();
        add_time(TECHNIQUE::K_CORE, start, end);
    #endif 

    //reduction of branching set B
    vector<Vertex*> branching_verticies = get_candidates(G);
    #ifdef BENCHMARK
        start = chrono::steady_clock::now();
    #endif
    branching_verticies = reduce_B_by_coloring(G.partial, branching_verticies, maximum_clique);
    #ifdef BENCHMARK
        end = chrono::steady_clock::now();
        add_time(TECHNIQUE::COLORING, start, end);
    #endif 

    //for all v ∈ B
    while(!branching_verticies.empty()){
        Vertex* branch_vertex = branching_verticies.back();
        branching_verticies.pop_back();

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
vector<Vertex*> branch_and_bound_mc(Graph& G){
    std::vector<Vertex*> maximum_clique;
    #ifdef BENCHMARK
        auto start = chrono::steady_clock::now();
    #endif
    branch_and_bound(G,maximum_clique);
    #ifdef BENCHMARK
        auto end = chrono::steady_clock::now();
        add_time(TECHNIQUE::BRANCH_AND_BOUND, start, end);
    #endif    
    
    return maximum_clique;
}
