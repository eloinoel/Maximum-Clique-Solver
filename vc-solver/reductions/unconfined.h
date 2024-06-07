#pragma once
#include "graph.h"

bool domination_rule(Graph& G);

bool domination_rule_known(Graph& G, Vertex* v);

bool unconfined_rule(Graph& G);
