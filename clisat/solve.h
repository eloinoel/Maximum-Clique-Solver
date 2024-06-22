#pragma once
#include "graph.h"
#include "ordering/degeneracy_ordering.h"
#include "pruning/ISEQ.h"
#include "find_max_clique.h"

void solve_clique(Graph& G);
void switch_to_subproblem(Graph& G, vector<Vertex*>& order, int i);