#pragma once
#include "graph.h"


vector<vector<Vertex*>> ISEQ(Graph& G, vector<Vertex*>& ordering);
vector<vector<Vertex*>> ISEQ_sets(Graph& G, vector<Vertex*>& ordering, int subproblem_K);
void ISEQ_color(Graph& G, vector<Vertex*>& ordering, int subproblem_K);
vector<vector<Vertex*>> isets_from_colors(Graph& G, vector<Vertex*>& ordering, int kappa);