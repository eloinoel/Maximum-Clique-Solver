#pragma once 
#include "graph.h"
#include "pruning/ISEQ.h"

/* FIXME: currently copying pruned, will need to change */
void find_max_clique(Graph& G, list<Vertex*> pruned);
void add_to_pruned(list<Vertex*>& pruned, Vertex* b);
bool check_kappa_partite(Graph& G, list<Vertex*>& branch);