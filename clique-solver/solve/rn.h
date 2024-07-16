#pragma once

#include <vector>
#include <string>

class Vertex;

/** 
 * Right neighbourhood struct, where we store whether 
 * a solution already exists and doesnt need to be recomputed
 */
typedef struct rn_{
    std::vector<Vertex*> neigh;
    int vc_size = 0;
    std::vector<std::string> sol;

    std::string to_string();
}rn;