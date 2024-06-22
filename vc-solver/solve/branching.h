#pragma once
#include "graph.h"

Vertex* max_deg_branching(Graph& G);

Vertex* min_amongN_branching(Graph& G);

vector<Vertex*> find_mirrors(Vertex* v, Graph& G);