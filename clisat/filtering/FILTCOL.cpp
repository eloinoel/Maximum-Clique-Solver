#include "FILTCOL.h"

#define FILTERED 2
#define SKIP     1 
#define NONE     0

vector<vector<Vertex*>> FILTCOL(Graph& G, int kappa, vector<int>& max_iset_member){
    //cout << "kappa " << kappa << "\n";
    //vector<int> max_iset_member(kappa+1);
    for(Vertex* v : G.V){
        v->data.iset.reference_class         = v->data.iset.iclass;
        assert(v->data.iset.iclass < kappa + 1);
        //max_iset_member[v->data.iset.iclass] = max(v->order_pos, max_iset_member[v->data.iset.iclass]);
        v->data.iset.iclass                  = -1;
        #if !NDEBUG 
            v->data.iset.filter_status           = NONE;
        #endif
    }


    vector<vector<Vertex*>> Isets(kappa); 

    list<Vertex*> v_list;
    for(Vertex* v : G.V)
        v_list.push_back(v);

    // this just does not work

    for(size_t i = 0; i < kappa; i++){
        auto iter_filt = v_list.begin();
        while(iter_filt != v_list.end()){
            Vertex* v        = *iter_filt;
            bool independent = true;
            for(Vertex* n : v->neighbors){
                if(n->data.iset.iclass == i){
                    independent = false;
                    break;
                }
            }

            if(!independent){
                iter_filt++;
                continue;
            }
            
            if(i != v->data.iset.reference_class){
                // filter v from graph
                if(v->order_pos > max_iset_member[i]){
                    #if !NDEBUG 
                        v->data.iset.filter_status = FILTERED;
                    #endif
                    iter_filt = v_list.erase(iter_filt);
                }else{
                //iter_filt = v_list.erase(iter_filt);
                    iter_filt++;
                }
                continue;
            }
        
            Isets[i].push_back(v);

            v->data.iset.iclass = i;
            iter_filt           = v_list.erase(iter_filt);
        }
    }

    //for(Vertex* b : v_list)
    //    Isets.back().push_back(b);

    assert(v_list.empty()); // everything processed
    return Isets;
}