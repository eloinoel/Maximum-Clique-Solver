#pragma once

#include "graph.h"

// void as 'deleting' things with deg0 rule doesn't actually change the Graph structure
void deg0_rule(Graph& G); 

bool deg1_rule(Graph& G);

void deg1_rule_single(Graph& G);

bool deg2_rule(Graph& G);

void deg2_rule_single(Graph& G);

bool deg3_rule(Graph& G);
