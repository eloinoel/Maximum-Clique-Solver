#pragma once

#include <vector>

using namespace std;

class Graph;
class Vertex;

vector<Vertex*> degeneracy_ordering(Graph& G);