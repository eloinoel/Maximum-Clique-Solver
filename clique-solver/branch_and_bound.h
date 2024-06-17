#pragma once

#include "upperbounds/upper_bounds.h"
#include "reductions/k_core.h"
#include "reductions/color_branching.h"

//Basic branch and bound framework for maximum clique
vector<Vertex*> branch_and_bound(Graph& G);