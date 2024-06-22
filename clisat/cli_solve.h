#pragma once

#include "ordering/degeneracy_ordering.h"
#include "pruning/ISEQ.h"
#include "find_max_clique.h"

class Graph;

void solve_clique(Graph& G);
void switch_to_subproblem(Graph& G, vector<Vertex*>& order, int i);