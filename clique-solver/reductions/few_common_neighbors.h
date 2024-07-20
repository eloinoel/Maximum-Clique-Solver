#pragma once 
#include "graph.h"

// bandaid fix for resolving clique with multiple solvers...
struct _recover_unit{
    string folded;
    string merged;
    string discard;

    void resolve(unordered_map<string, state>& sol){
        if(sol.find(merged) == sol.end() || sol[merged] != CLIQUE){
            sol[merged] = EXCLUDED;
            sol[discard] = EXCLUDED;
            sol[folded] = CLIQUE;
        }else{
            assert(sol[merged] == CLIQUE);
            sol[merged] = CLIQUE;
            sol[discard] = CLIQUE;
            sol[folded] = EXCLUDED;
        }
    }
};

vector<string> reduce(Graph& G,  unordered_map<string, state>& sol, vector<_recover_unit>& rec);

void zero_dependencies(Graph& G);

vector<string> get_LB_wrapper(Graph& G);

void few_common_rule(Graph& G, int LB = -1);

void sparse_neighbor_rule(Graph& G, int LB = -1);

void domination_comp_rule(Graph& G);

void universal_rule(Graph& G, unordered_map<string, state>& sol);

void comp_deg1_rule(Graph& G, unordered_map<string, state>& sol);

void comp_deg2_rule(Graph& G, unordered_map<string, state>& sol, vector<_recover_unit>& rec);