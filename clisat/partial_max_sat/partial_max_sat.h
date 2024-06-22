#pragma once
#include "graph.h"
#include "pruning/ISEQ.h"

/* fun idea but dont want to bother atm
using Literal = Vertex*;
using Clause = vector<Literal>;
*/

typedef struct Clause{
    /* the vectors are intentionally not references because we need to copy/reuse an old version multiple times */
    vector<Vertex*> literals;
    vector<int> slack; // this represents the amount of slack variables, als well as the index of a slack constraint to 'use up' that variable
    int slack_remaining = 0;     // remaining amount of slack_variables
    bool active = true; // TODO: fix lazy delete
};

class Partial_Max_Sat{
public:

    /* hard clauses are implicit 
    simply iterating all soft clauses (the independent sets) and delete all literals that correspond to non-adjacent vertices
    if a soft clause is empty, we have a conflict
    */
    //vector<vector<Vertex*>> soft_clauses;
    vector<Clause> soft_clauses;
    vector<Vertex*> branchset;

    /* this is used for z1 + ... + zn = 1
     *  first int: idx of clause containing this slack variable
     * second int: idx of that clauses' slack referencing back to this
     * when a slack is used by clause C:
     *  - go to C.slack, take first int := i 
     *  - go to cstr := slack_constraint[i] 
     *  - for each s in cstr: 
     *  -   go to CS = Clause[s.first]
     *  -   set CS.slack[s.second] = -1 //TODO: fix lazy delete
     * idea is pretty much the same as the edge/neighbor thing used in graph 
     */
    vector<vector<pair<int, int>>> slack_constraint; 

    Partial_Max_Sat(vector<vector<Vertex*>>& iset_instance){
        branchset = move(iset_instance.back());
        iset_instance.pop_back();

        soft_clauses.reserve(iset_instance.size());
        //soft_clauses = isat_instance;
        for(auto iset : iset_instance){
            soft_clauses.push_back({move(iset), {}});
        }
    };

    list<Vertex*> SATCOL(Graph& G);
    bool detect_conflict_iset(Graph& G,vector<Vertex*>& b_iset);

    list<Vertex*> output_pruned_to_list(); //FIXME: this is really ugly

    // could just make this use a vector<Vertex*> since all the units are just a Vertex anyways? saves one indirection
    vector<int> propagate_units(Graph& G, vector<int>& units);

    int use_slack(Clause& C);




};