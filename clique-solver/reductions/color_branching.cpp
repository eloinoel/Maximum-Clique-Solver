#include "color_branching.h"

/**
 * @param coloring 
 * @param k 
 * @param v 
 * @return true if any n ∈ N(v) has color(n) == k
 * @return false if no n ∈ N(v) has color(n) == k
 */
inline bool is_color_in_neighborhood(unordered_map<Vertex*, int> coloring, int k, Vertex* v){
    for(Vertex* u: v->neighbors){
        if(coloring[u]==k){
            return true;
        }
    }
    return false;
}

/**
 * @brief collects all n ∈ N(v) with color(n) == k
 * 
 * @param coloring 
 * @param k color
 * @param v vertex
 * @return { n |  n ∈ N(v) with color(n) == k }
 */
inline vector<Vertex*> neighbors_of_color_k(unordered_map<Vertex*, int> coloring, int k, Vertex* v){
    vector<Vertex*> neighbors;
    for(Vertex* n: v->neighbors){
        if(coloring[n]==k){
            neighbors.push_back(n);
        }
    }
    return neighbors;
}

/**
 * @brief try to switch color(v) with color(n) for any n ∈ N(v)
 * 
 * @param coloring 
 * @param k 
 * @param v
 * @param max_color 
 * @return color(v) after the try
 */
inline int renumber(unordered_map<Vertex*, int> coloring, int k, Vertex* v, int max_color){

    for(int l = 1; l<=max_color; l++){
    
        //check for |N(v) ∩ C_l| == 1
        vector<Vertex*> neighbors = neighbors_of_color_k(coloring,l, v);
        if(neighbors.size()!=1){
            continue;
        }
            
        //check for |N(n) ∩ C_k| == 0 for the one n ∈ N(v) with color(n) == l
        Vertex* neighbor = neighbors.back();
        if(neighbors_of_color_k(coloring, k, neighbor).size()!=0){
            continue;
        }

        //swap color(n) with color(v)
        coloring[v]=l;
        coloring[neighbor]==k;
        return l;
    }

    return k;
}

vector<Vertex*> reduce_B_by_coloring(vector<Vertex*> partial_clique, vector<Vertex*> branching_candidates, vector<Vertex *> maximum_clique){
    //preferred max color
    int preferred_max_color = maximum_clique.size()-partial_clique.size();

    vector<Vertex*> new_branching_verticies;
    unordered_map<Vertex*, int> coloring;

    for(Vertex* candidate: branching_candidates){

        //basic coloring 
        int color = 1;
        while(is_color_in_neighborhood(coloring,color,candidate)==true){
            color++;
        }
        coloring[candidate]=color;

        //try to switch color(v) with color(n) for any n ∈ N(v) 
        if(color > preferred_max_color){
            color = renumber(coloring,color,candidate, preferred_max_color);
        }

        if(color > preferred_max_color){
            new_branching_verticies.push_back(candidate);
        }
    }

    return new_branching_verticies;     
}