#include <utility>
#include <unordered_map>
#include "graph.h"
#include "degeneracy_ordering.h"
#include "color_branching.h"
#include "inc_max_sat.h"

class Vertex;

//only used intern
struct Container{
    vector<Vertex*> maximum_clique;
    int max_core;
    vector<Vertex*> ordering;

    Container(vector<Vertex*> mclique, int mcore, vector<Vertex*> o):maximum_clique(mclique),max_core(mcore), ordering(o){};
};
//index of next vertex in G with min degree. has last index as argument to make it efficient
inline int next(int befor, Graph& G){
    for(int i = max(0, befor-1); i<G.deg_lists.size(); i++){
        if(!G.deg_lists[i].empty())return i;
    }
    return -1;
}

/**
 * @brief basically a lower bound, the corresponding core and new ordering are created
 * 
 * @param G 
 * @param lb 
 * @return C_max, max_core, ordering
 */
Container MM_initialize(Graph& G, int lb){
    unordered_map<Vertex*, int> core_number;
    int N = 0;
    for(auto l: G.deg_lists)N+=l.size();

    if(N==0)return Container({},0,{});
    G.set_restore();
    int cur_core = next(0,G);

    vector<Vertex*> ordering;
    vector<Vertex*> C_0;
    ordering.reserve(N);
    int i = cur_core;
    int k = 0;
    for(; i!=-1;i=next(i,G)){
        k++;
        Vertex* v = G.deg_lists[i].back();
        
        if(i > cur_core){
            cur_core=i;
        }
        core_number[v]=cur_core;
        if(i == N-k){
            for(auto l: G.deg_lists){
                ordering.insert(ordering.end(), l.begin(), l.end());
                C_0.insert(C_0.end(), l.begin(), l.end());
                for(auto e:l){
                    core_number[e]=cur_core;
                }
            }
            break;
        }
        ordering.push_back(v);
        G.MM_discard_vertex(v);
    }
    G.restore();

    if(C_0.size()>lb){
        lb=C_0.size();
    }
    vector<Vertex*> new_ordering;
    int max_core = -1;
    for(Vertex* v : ordering){
        max_core = max(max_core, core_number[v]);
        if(core_number[v]<lb){
            G.MM_discard_vertex(v);
        }else {
            new_ordering.push_back(v);
        }
    }
    return Container(C_0,max_core,new_ordering);
}

/**
 * @brief BnB + DSATUR + IncMaxSat
 * 
 * @param G 
 * @param C_max 
 * @param ordering 
 */
void search_max_clique(Graph& G, vector<Vertex*>& C_max, vector<Vertex*>& ordering){
    
    if(G.partial.size()>C_max.size()){
        C_max = G.partial;
    }

    //branch reduction (create B)
    vector<Vertex*> B = ordering;
    int r = C_max.size()-G.partial.size();
    if(r>0){
        Color_container con = dsatur(G, r, ordering);
        B = inc_max_sat(con.new_ordering, con.coloring, r);
        ordering = con.new_ordering;
    }
    if(B.empty())return;

    //create A = V \ B
    vector<Vertex*> A;
    A.reserve(ordering.size() - B.size());
    int b = 0;
    for(int i=0; i<ordering.size();i++){
        if(ordering[i]==B[b]){
            b++;
        }else{
            A.push_back(ordering[i]);
        }
    }

    vector<Vertex*> P;
    for(int i = ordering.size()-1; i>=0; i--){
        //P' = N(v_i) ∩ {v_i+1, ... v_n}
        P.clear();
        for(int k = i+1; k<ordering.size(); k++){
            if(ordering[i]->adjacent_to(ordering[k])){
                P.push_back(ordering[k]);
            }
        }

        G.set_restore();
        G.MM_clique_add_vertex(ordering[i]);
        G.MM_induced_subgraph(P);
        
        //BnB(C', C_max, P')
        search_max_clique(G, C_max, P);
        G.restore();
    }
}

/**
 * @brief preprocessing, then BnB with DSATUR and IncMaxSat
 * 
 * @param G 
 * @return maximum clique 
 */
vector<Vertex*> lmc(Graph& G){
    vector<Vertex*> maximum_clique;
    G.set_restore();

    //1. preprocessing
    Container container = MM_initialize(G, 0);
    vector<Vertex*> C_max = container.maximum_clique;
    int max_core = container.max_core;
    vector<Vertex*> O = container.ordering;
    if(C_max.size()==max_core+1)return C_max;

    vector<Vertex*> P;
    for(int i = O.size()-1; i>=0; i--){
        //P = N(v_i) ∩ {v_i+1, ... v_n}
        P.clear();
        for(int k = i+1; k<O.size(); k++){
            if(O[k]->adjacent_to(O[i])){
                P.push_back(O[k]);
            }
        }

        G.set_restore();
        G.MM_induced_subgraph(P);
        G.MM_clique_add_vertex(O[i]);

        //2. preprocessing
        Container container = MM_initialize(G, C_max.size()-1);
        if(container.maximum_clique.size()>= C_max.size()){
            C_max.assign(container.maximum_clique.begin(),container.maximum_clique.end());
            C_max.push_back(O[i]); 
        }
        if(container.max_core +1 >= C_max.size()){
            
            //BnB with DSATUR and IncMaxSat
            search_max_clique(G, C_max, container.ordering);
        }
        G.restore();
    }
    G.restore();
    return C_max;
}