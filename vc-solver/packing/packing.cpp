#include "packing.h"

#define not_deleted(x) (x->list_idx != (size_t) -1)

int Packing_Constraint::evaluate(Graph& G){
    int sum = 0;
    int unknown = 0;

    for(Vertex* xv : vars){
        sum     += (xv->data.resolved_status == VC);
        unknown += (xv->status == UNKNOWN && not_deleted(xv));
    }

    if(sum > max)
        return -1;
    
    bool reduced = false;
    if(sum == max && unknown > 0){
        vector<Vertex*> S; S.reserve(unknown);
        G.new_timestamp();
        for(Vertex* xv : vars){
            if(xv->status == UNKNOWN && not_deleted(xv)){
                S.push_back(xv);
                xv->data.packing.count = -1;
                xv->marked = G.timestamp;
            }
        }

        for(Vertex* xs : S){
			for(Vertex* n : xs->neighbors){
                if(n->marked != G.timestamp){
                    n->data.packing.count = 1;
                    n->marked = G.timestamp;
                }else if(n->data.packing.count == -1){
                   return -1; 
                }
                else{
                    n->data.packing.count++;
                }
            }
        }
        for(Vertex* xs : S){
            for(Vertex* n : xs->neighbors){
                if(n->data.packing.count == 1){
                    vector<Vertex*> new_vars;
                    for(Vertex* w : n->neighbors){
                        if(w->marked != G.timestamp)
                            new_vars.push_back(w);
                    }
                    G.constraints.push_back(new Packing_Constraint(new_vars, new_vars.size()-1));
                }
            }
        }
        for(Vertex* xs : S){
            assert(not_deleted(xs) && xs->status == UNKNOWN);
            G.MM_vc_add_neighbors(xs);
        }
        reduced = true;
    }
    else if(sum + unknown > max){
        vector<Vertex*> S; S.reserve(unknown);
        G.new_timestamp();
        for(Vertex* xv : vars){
            if(xv->status == UNKNOWN && not_deleted(xv)){
                S.push_back(xv);
                xv->marked = G.timestamp;
            }
        }
        for(Vertex* v : S[0]->neighbors){
            if(v->marked != G.timestamp){
                int count = 0;
                for(Vertex* u : v->neighbors)
                    count += (u->marked == G.timestamp);
                
                if(sum + count > max){
                    vector<Vertex*> new_vars; new_vars.reserve(deg(v));
                    for(Vertex* u : v->neighbors)
                        new_vars.push_back(u);
                    
                    G.constraints.push_back(new Packing_Constraint(new_vars, new_vars.size()-2));
                    G.MM_vc_add_vertex(v);
                    reduced = true;
                    break;
                }
            }
        }

    }
    return (int) reduced;
}

void add_selected_constraint(Graph& G, Vertex* v,  int c){
    G.constraints.push_back(new Packing_Constraint(v->neighbors, deg(v) - c));    
}

void add_nomirror_constraint(Graph& G, Vertex* v){
    G.new_timestamp();
    v->marked = G.timestamp;
    for(Vertex* u : v->neighbors)
        u->marked = G.timestamp;
    
    for(size_t i = G.deg_lists.size() - 1; -- i != 0;){
        for(Vertex* v : G.deg_lists[i]){
            v->data.found = nullptr;
        }
    }


    for(Vertex* u : v->neighbors){

        bool stronger = false;
        vector<Vertex*> vars;
        for(Vertex* w : u->neighbors){
            if(w->marked != G.timestamp){
                vars.push_back(w);
                w->data.found = u;
            }
        }

        assert(vars.size() > 0 && "vars size > 0");

        for(Vertex* u2 : vars[0]->neighbors){
            if(u2->marked == G.timestamp && u2 != u){
                int count = 0;
                for(Vertex* w : u2->neighbors){
                    if(w->data.found == u){
                        count++;
                    }else if(w == u || w->marked != G.timestamp){
                        count = -1;
                        break;
                    }
                }
            
            if(count == (int) vars.size()){
                stronger = true;
                break;
            }
            }
        }
        int off = (stronger) ? 2 : 1;
        G.constraints.push_back(new Packing_Constraint(vars, (int)(vars.size()) - off));
    }
}

void add_not_selected_constraint(Graph& G, Vertex* v){
    G.new_timestamp();
    v->marked = G.timestamp;
    for(Vertex* u : v->neighbors) 
        u->marked = G.timestamp;
    // N[v] is marked

    for(Vertex* w: v->neighbors){
        vector<Vertex*> w_N_plus;
        for(Vertex* u : w->neighbors){
            if(u->marked != G.timestamp){
                w_N_plus.push_back(u);
            }
        }
    
        if(w_N_plus.size() > 0){
            G.constraints.push_back(new Packing_Constraint(w_N_plus, w_N_plus.size() - 1));
        }
    }
}
int packing_rule(Graph& G){
    G.resolve_vc(true);
    bool reduced = false;
    for(Packing_Constraint* P : G.constraints){
        int eval = P->evaluate(G);
        if(eval == -1)
            return -1;
        reduced |= eval;       
    }
    return reduced;
}