#include "debug.h"

void print_buckets(Graph& G){
    cout << "degree buckets: \n";
    for(int i = static_cast<int>(G.deg_lists.size())-1; i >= 0; i--){
        if(!G.deg_lists[i].empty()){
            cout << i << ": " << G.deg_lists[i].size() << "\n";
        }
    }
}

void basic_consistency_check(Graph& G, bool reset){
    #if !DEBUG
        cout << "set the debug flag to use this check\n";
    #else 
        if(reset){
            G.deg_list_state.clear();
            return;
        }
        vector<pair<size_t, size_t>> state;
        for(int i = static_cast<int>(G.deg_lists.size())-1; i >= 0; i--){
            if(!G.deg_lists[i].empty()){
                state.push_back({i, G.deg_lists[i].size()});
            }
        }

        if (G.deg_list_state.size() != 0){
            assert(state.size() == G.deg_list_state.size());
            for(size_t i = 0; i < state.size(); i++){
                assert(state[i] == G.deg_list_state[i]);
                cout << "true\n";
            }
        }
        else{
            G.deg_list_state = move(state);
        }

    #endif
}