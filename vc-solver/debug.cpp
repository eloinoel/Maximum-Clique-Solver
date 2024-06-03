#include "debug.h"

void print_buckets(Graph& G){
    cout << "degree buckets: \n";
    for(int i = static_cast<int>(G.deg_lists.size())-1; i >= 0; i--){
        if(!G.deg_lists[i].empty()){
            cout << i << ": " << G.deg_lists[i].size() << "\n";
        }
    }
}