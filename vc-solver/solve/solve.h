#pragma once
#include "graph.h"

bool vc_branch(Graph& G);

size_t solve_k(Graph& G);

int solve(Graph& G);

pair<bool, int> solve_limitUB(Graph& G, int UB);

void branch(Graph& G); 