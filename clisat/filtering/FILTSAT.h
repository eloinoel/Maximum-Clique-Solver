#pragma once
#include "graph.h"
#include "pruning/ISEQ.h"
#include <set> //temp...
/* fun idea but dont want to bother atm
using Literal = Vertex*;
using Clause = vector<Literal>;
*/

typedef struct Filt_Clause{
    /* the vectors are intentionally not references because we need to copy/reuse an old version multiple times */
    vector<Vertex*> literals;
    bool active = true; // TODO: fix lazy delete
};

/* yes... this is just partial_max_sat copy&paste without slack variables*/
class FILTSAT{
public:

    /* hard clauses are implicit 
    simply iterating all soft clauses (the independent sets) and delete all literals that correspond to non-adjacent vertices
    if a soft clause is empty, we have a conflict
    */
    //vector<vector<Vertex*>> soft_clauses;
    vector<Filt_Clause> soft_clauses;

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

    FILTSAT(vector<vector<Vertex*>>& iset_instance){
        soft_clauses.reserve(iset_instance.size());
        //soft_clauses = isat_instance;
        for(auto iset : iset_instance){
            soft_clauses.push_back({move(iset), {}});
        }
    };

    list<Vertex*> filter(Graph& G, bool& cut);
    bool detect_conflict_iset(Graph& G, vector<Vertex*>& b_iset, vector<int>& to_filter);

    list<Vertex*> output_pruned_to_list(); //FIXME: this is really ugly

    // could just make this use a vector<Vertex*> since all the units are just a Vertex anyways? saves one indirection
    bool propagate_units(Graph& G, vector<int>& units);
};