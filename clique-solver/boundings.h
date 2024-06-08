#include "../vc-solver/graph.h"

enum class Bounding_status: bool{
    CUTOFF      = true,
    NO_CUTOFF   = false
};

Bounding_status bounding1(Graph& G, vector<Vertex*>& maximum_clique);
