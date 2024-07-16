#include "graph.h"
#include "rn.h"

using namespace std;

string rn_::to_string()
{
    string str = "";
    for(auto v : neigh)
    {
        std::cout << v->id << " ";
        str += v->id + " ";
    }
    return str; 
}
