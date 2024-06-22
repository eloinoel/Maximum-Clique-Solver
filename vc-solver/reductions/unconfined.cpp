#include "unconfined.h"
#include "deg_rules.h"

bool domination_rule_single(Graph& G, Vertex* v){
    G.new_timestamp();

    for(Vertex* n : v->neighbors){
        n->marked = G.timestamp;
    }

    v->marked = G.timestamp;

    // check if v dominates a vertex n
    for(Vertex* n : v->neighbors){
        if(deg(n) > deg(v)){
            continue;
        }

        for(Vertex* w : n->neighbors){
            if(w->marked != G.timestamp){
                goto fail;
            }
        }

        //here, n is dominated by v
        G.MM_vc_add_vertex(v);
        return true;

        fail:
        continue;
    }

    return false;
}

bool domination_rule_known(Graph& G, Vertex* v){
    G.new_timestamp();
    for(Vertex* n : v->neighbors){
        n->marked = G.timestamp;
    }

    v->marked = G.timestamp;

    // check which neighbor dominates v
	for (Vertex* u : v->neighbors) {
		if (deg(u) < deg(v)) {
			continue;
		}

        size_t count = 0;
        int max_skip = static_cast<int>(deg(u) - deg(v));
		for (Vertex* n : u->neighbors) {        
            max_skip -= (n->marked != G.timestamp);
            if(max_skip < 0){
                break;
            }
            count += (n->marked == G.timestamp);

            if(count == deg(v)){
                // u dominates v
                G.MM_vc_add_vertex(u);
                return true;
            }
		}
    }   
    
    return false;

}

bool domination_rule(Graph& G){
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
            if(deg(v) > 0 && v->status == UNKNOWN){
                if(domination_rule_single(G, v)){
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
                    if(i + 1 == count)
                        goto full;
                }
            }
            full:
            continue;
        }
    }while(reduced);
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced_once;
}


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
    int p = 1;
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
                goto cleanup_unconf;
             }
             else if (size == 1){ // exactly one w
                S.push_back(NU_dif_NS);
                NU_dif_NS->marked = mark;
                added_to_S = true;
                p++;
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
    if(p >= 2){
        struct cand{
            Vertex* u;
            Vertex* v1;
            Vertex* v2;
            
        };
        vector<cand> NS_deg2;
        for(Vertex* x : NS){
            if(x->marked == mark + 1)
                NS_deg2.push_back({x, nullptr, nullptr});
        }
        for(Vertex* s : S) 
            s->marked = mark-1;
    
        for(Vertex* n : NS)
            n->marked = mark;
        
        size_t i = 0;
        restart:
        while(i < NS_deg2.size()){
            auto& c = NS_deg2[i];
            for(Vertex* w : c.u->neighbors){
                if(w->marked < mark){
                    if(c.v1 == nullptr){
                        c.v1 = w;
                    }else if(c.v2 == nullptr){
                        c.v2 = w;
                    }else{
                        swap(NS_deg2.back(), NS_deg2[i]);
                        NS_deg2.pop_back();
                        goto restart;
                    }
                }
            }
            if(c.v1 == nullptr || c.v2 == nullptr){
                swap(NS_deg2.back(), NS_deg2[i]);
                NS_deg2.pop_back();
                goto restart;
            }
            i++;
            if(c.v1 > c.v2)
                swap(c.v1, c.v2);


            bool found1 = false;
            for(Vertex* s : S)
                found1 |= (s == c.v1);
            
            bool found2 = false;
            for(Vertex* s : S)
                found2 |= (s == c.v2);

            assert(found1 && found2);

            int count_s = 0;
            for(Vertex* x : c.u->neighbors){
                for(Vertex* s : S)
                    count_s += (s == x);
            }
            
            assert(count_s == 2);
            
        }
        
        for(size_t ii = 0; ii < NS_deg2.size(); ii++){
            auto& c = NS_deg2[ii];
            for(size_t j = ii + 1; j < NS_deg2.size(); j++){
                auto& c2 = NS_deg2[j];
                if(c.v1 == c2.v1 && c.v2 == c2.v2){
                    //success
                    if(c2.u->adjacent_to(c.u))
                        continue;

                    for(Vertex* x : c.u->neighbors){
                        if(x->marked != G.timestamp){
                            assert(x == c.v1 || x == c.v2);
                        }
                    }

                    for(Vertex* x : c2.u->neighbors){
                        if(x->marked != G.timestamp){
                            assert(x == c.v1 || x == c.v2);
                        }
                    }
                   
                    unconfined = true;
                    goto cleanup_unconf;
                }
            }
        }
    }

    cleanup_unconf:
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
            if(deg(v) > 0 && v->status == UNKNOWN){
                if(is_unconfined(G, v)){
                    G.MM_vc_add_vertex(v);
                    reduced = true;
                    reduced_once = true;
                }
            }
        }
        deg0_rule(G);
        //prevent having to clear the vector, reuse memory
        if(reduced){
            count = G.N;
            size_t i = 0;
            for(size_t j = G.deg_lists.size() - 1; -- j != 0;){
                for(Vertex* v : G.deg_lists[j]){
                    V[i] = v;
                    i++;
                    if(i + 1 == count)
                        goto full;
                }
            }
            full:
            continue;
        }
    }while(reduced);

    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced_once;
}