#include "unconfined.h"

bool is_unconfined(Graph& G, Vertex* v){
    unsigned long long mark = G.timestamp;
    //std::cout << mark << '\n';
    bool unconfined = false;
    std::vector<Vertex*> S;
    std::vector<Vertex*> NS;

    S.push_back(v); // S = {v}
    v->marked = mark;

    for(auto& n: v->neighbors){
        NS.push_back(n);
        n->marked = mark;
    } // NS = N(S) = N(v)

    bool added_to_S = true;

    while(added_to_S){
        added_to_S = false;
        for(size_t i = 0; i < NS.size(); i++){ // find u in N(S) [Note: can't use iterator since vector might realloc]
            Vertex* u = NS[i];

            if(u->marked != mark) continue; // |N(u) intersect S| = 1

            Vertex* NU_dif_NS = nullptr;
            size_t size = 0;

            for(auto& w: u->neighbors){
                if(w->marked < mark){
                    NU_dif_NS = w;
                    size++;
                    if(size > 1) break;
                }
            }

            if(size == 0){ // emptyset
                unconfined = true;
                goto cleanup_uncof;
             }
             else if (size == 1){ // exactly one w
                S.push_back(NU_dif_NS);
                NU_dif_NS->marked = mark;
                added_to_S = true;

                for(auto& w : NU_dif_NS->neighbors){
                    if(w->marked < mark){
                        w->marked = mark;
                        NS.push_back(w);
                    }
                    else{
                        w->marked++;
                    }
                }
            }
        }
    }
    // diamond reduction
   // if (S.size() > 2){
   //     
   // }

    cleanup_uncof:
    for(auto& s : S) 
        s->marked = mark-1;
    
    for(auto& n : NS)
        n->marked = mark-1;

    return unconfined;

}


bool unconfined_rule(Graph& G){
    G.new_timestamp();
    vector<Vertex*> V;
    for(size_t i = G.deg_lists.size() - 1; -- i != 0;){
        for(Vertex* v : G.deg_lists[i]){
            V.push_back(v);
            assert(v->status == UNKNOWN);
        }
    }
    size_t count = V.size();
    bool reduced = false;
    bool reduced_once = false;
   
    do{
        reduced = false;
        for(size_t i = 0; i < count; i++){
            Vertex* v = V[i];
            if(v->degree() > 0 && v->status == UNKNOWN){
                if(is_unconfined(G, v)){
                    G.MM_select_vertex(v);
                    reduced = true;
                    reduced_once = true;
                }
            }
        }
        //prevent having to clear the vector, reuse memory
        if(reduced){
            count = G.N;
            size_t i = 0;
            for(size_t j = G.deg_lists.size() - 1; -- j != 0;){
                for(Vertex* v : G.deg_lists[j]){
                    V[i] = v;
                    i++;
                }
            }
        }
    }while(reduced && G.k > 0);

    return reduced_once;
}