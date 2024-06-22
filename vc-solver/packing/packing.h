#pragma once 
#include "graph.h"

class Graph;
class Vertex;

class Packing_Constraint{
private:
    vector<Vertex*> vars;
    int max = numeric_limits<int>::max();

public:
    int evaluate(Graph& G);

    //void reduce(Graph& G);
    Packing_Constraint(vector<Vertex*> vars_, int max_)
     : vars(vars_), max(max_){
        //assert(max >= 0);
     }
};


void add_selected_constraint(Graph& G, Vertex* v, int c = 1);
void add_nomirror_constraint(Graph& G, Vertex* v);
void add_not_selected_constraint(Graph&G, Vertex* v);

int packing_rule(Graph& G);