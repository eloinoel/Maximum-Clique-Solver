#pragma once

#include "graph.h"
#include <algorithm>

int basic_clique_cover(Graph& G);

int block_clique_cover(Graph& G);

// require having executed block_clique_cover first to fill vertex data.block
int iterated_greedy_clique(Graph& G);
int iterated_greedy_clique_full_permutes(Graph& G);
int iterated_greedy_clique_full_permutes_class_index(Graph& G);