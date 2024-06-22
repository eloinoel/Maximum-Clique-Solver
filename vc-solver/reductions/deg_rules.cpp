#include "deg_rules.h"
#include "unconfined.h"

void deg0_rule(Graph& G){
    vector<Vertex*> deleted;
    while(!G.deg_lists[0].empty()){
        Vertex* v = G.deg_lists[0].back();
        deleted.push_back(v);
        G.deg_lists[0].pop_back();
        assert(v->status == UNKNOWN);
        v->status = EXCLUDED;
        G.N--;
        v->deg_idx = -1;
        v->list_idx = -1;
    }
    if(!deleted.empty())
        G.history.emplace_back(new OP_DeleteDeg0(deleted));
}

void deg1_rule_single(Graph& G){
    Vertex* v = G.deg_lists[1].back();
    assert(deg(v) == 1);
    G.MM_vc_add_neighbors(v);
        
}

bool deg1_rule(Graph& G){
    bool reduced = false;
    while(G.max_degree >= 1 &&!G.deg_lists[1].empty()){
        deg1_rule_single(G);
        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;
}


class VC_deg2_fold : public VC_Transformation{
public:
    Vertex* folded;     // folded, the deg2 vertex - 'v'
    Vertex* discard;    // discarded, the vertex that will not stay after merge 
    Vertex* merged;     // merged, the vertex that gets merged with discard and stays in the grap

    vector<Edge*> moved_edges;
    
    // this pattern by aleksander of executing the rule while creating the operation is pretty neat I think
    VC_deg2_fold(Vertex* folded, Graph& G)
     : folded(folded) {
        G.transform_access.push_back(this);
        G.delete_vertex(folded); //note: state stays UNKNOWN

        discard = folded->neighbors[0];
        merged = folded->neighbors[1];

        //discard has smaller neighborhood
        if(deg(discard) > deg(merged))
            swap(discard,merged);

        G.new_timestamp();

        for(Vertex* n : merged->neighbors)
            n->marked = G.timestamp;
        
        for(Vertex* n : discard->neighbors){
            // not common edge, add to b
            if(n->marked != G.timestamp){
                Edge* e = G.add_edge(merged,n);
                moved_edges.push_back(e);
            }
        }

        G.delete_vertex(discard);

        G.k--;
        G.sol_size++;
    }

    void undo (Graph* G) const{
        G->transform_access.pop_back();
        G->sol_size--;
        G->k++;
        G->restore_vertex(discard);
        for(auto it = moved_edges.rbegin(); it != moved_edges.rend(); it++){
            G->delete_edge(*it);
        }
        G->restore_vertex(folded);
    }

    void resolve(Graph* G, bool partial) {
        assert(folded->data.resolved_status == UNKNOWN);
        assert(discard->data.resolved_status == UNKNOWN);
        
        if(merged->data.resolved_status == VC){
            discard->data.resolved_status = VC;
            folded->data.resolved_status = EXCLUDED;
            return;
        }
        if(merged->data.resolved_status == EXCLUDED){
            discard->data.resolved_status = EXCLUDED;
            folded->data.resolved_status = VC;
            return;
        }
        if(partial)
            return;
        assert(false && "error deg2");
    }

    void debug_print(Graph* G, bool partial){
        cout << "#### DEG 2 FOLD\n";
        state fs = folded->data.resolved_status;
        state ds = discard->data.resolved_status;
        state ms = merged->data.resolved_status;
        resolve(G, partial);
        state after_fs = folded->data.resolved_status;
        state after_ds = discard->data.resolved_status;
        state after_ms = merged->data.resolved_status;

        G->print_op_line(folded , "folded" , fs, after_fs);
        G->print_op_line(discard, "discard", ds, after_ds);
        G->print_op_line(merged , "merged" , ms, after_ms);
        cout <<"#### DONE\n";
    }
};

void deg2_rule_single(Graph& G){
    Vertex* v = G.deg_lists[2].back();
    assert(deg(v) == 2);

    Vertex* a = v->neighbors[0];
    Vertex* b = v->neighbors[1];

    assert(deg(a) > 0);
    assert(deg(b) > 0);

    if(a->adjacent_to(b)){
        //triangle case
        G.MM_vc_add_neighbors(v);
    }
    else{
        G.history.emplace_back(new VC_deg2_fold(v,G));
    }

    return;

}

bool deg2_rule(Graph& G){
    bool reduced = false;
    while(G.max_degree >= 2 && !G.deg_lists[2].empty()){
        deg2_rule_single(G);
        deg1_rule(G); //ensures min deg > 1, otherwise need to special case deg = 1 to avoid unnecessary folds
        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;
}

class VC_deg3_IS : public VC_Transformation{
public: 
    Vertex *v;
    Vertex* IS[3]; // independset set = N(v)
    vector<Edge*> edges_added;
    Edge* connect_IS[2];

    VC_deg3_IS(Vertex* v, Graph& G)
     : v(v) {
        G.transform_access.push_back(this);
        for(size_t i = 0; i < 3; i++)
            IS[i] = v->neighbors[i];

        G.delete_vertex(v);

        vector<Vertex*> to_add[3];

        for(size_t i = 0; i < 3; i++){
            size_t j = (i+1) % 3;
            G.new_timestamp();

            for(Vertex* s : IS[i]->neighbors)
                s->marked = G.timestamp;  

            // watch out for common neighbors
            for(Vertex* n: IS[j]->neighbors){
                if(n->marked != G.timestamp)
                    to_add[i].push_back(n);
            } 
         }

        for(size_t i = 0; i < 3; i++){
            for(Vertex* x : to_add[i]){
                Edge* e = G.add_edge(IS[i], x);
                edges_added.push_back(e);
            }
        }

        connect_IS[0] = G.add_edge(IS[0], IS[1]);
        connect_IS[1] = G.add_edge(IS[1], IS[2]);
     }

     void undo(Graph* G) const{
        G->transform_access.pop_back();
        G->delete_edge(connect_IS[1]);
        G->delete_edge(connect_IS[0]);

        for(auto it = edges_added.rbegin(); it != edges_added.rend(); ++it){
            G->delete_edge(*it);
        }

        G->restore_vertex(v);
     }

     void resolve(Graph* G, bool partial){
        assert(v->data.resolved_status == UNKNOWN);
        size_t count = 0;
        size_t unknown = 0;
        for(size_t i = 0; i < 3; i++){
            count += IS[i]->data.resolved_status == VC;
            unknown += IS[i]->data.resolved_status == UNKNOWN;
        }

        if(unknown > 0 && partial){
            return;
        }

        if(count == 3){
            assert(v->data.resolved_status == UNKNOWN);
            v->data.resolved_status = EXCLUDED;
        }
        else if(count == 2){
            size_t i;   // find the S[i] not in solution
            for(i = 0; i < 3; i++){
                if(IS[i]->data.resolved_status != VC)
                    break;
            }
            IS[(i+1)%3]->data.resolved_status = EXCLUDED;
            if(unknown != 0){
                assert(unknown == 1);
                v->data.resolved_status = UNKNOWN;
            }else{
                v->data.resolved_status = VC;
            }
        }else if(count == 1 && unknown == 0){
            // only S[1] should be possible as it is the one connecting the IS
            assert(IS[1]->data.resolved_status == VC);
            IS[1]->data.resolved_status = EXCLUDED;
            v->data.resolved_status = VC;
        }else{
            assert(partial); 
            v->data.resolved_status = UNKNOWN;
            for(Vertex* s : IS)
                s->data.resolved_status = UNKNOWN;
        }
     }

     void debug_print(Graph* G, bool partial){
        cout << "#### DEG 3 IS\n";
        state vs = v->data.resolved_status;
        state befores[3];
        for(size_t i = 0; i < 3; i++){
            befores[i] = IS[i]->data.resolved_status;
        }
        resolve(G, partial);
        state after_vs = v->data.resolved_status;
        state afters[3];
        for(size_t i = 0; i < 3; i++){
            afters[i] = IS[i]->data.resolved_status;
        }

        G->print_op_line(v , "v" , vs, after_vs);
        for(size_t i = 0; i < 3; i++){
            G->print_op_line(IS[i], "IS - " + to_string(i), befores[i], afters[i]);
        }
        cout <<"#### DONE\n";
     }
};


class VC_deg3_CN : public VC_Transformation{
public:
    Vertex* v;
    pair<Vertex*, Vertex*> C1; // clique with 2 members
    Vertex* C2; // 'clique' with 1 member
    vector<Edge*> edges_added;

    VC_deg3_CN(Vertex* v_, pair<Vertex*, Vertex*> C1_,  Vertex* C2_, Graph& G)
     : v(v_), C1(C1_), C2(C2_){
        G.delete_vertex(v);
        G.transform_access.push_back(this);
        assert(C1.first->adjacent_to(C1.second));
        Vertex* C1_iter[2] = {C1.first, C1.second};

        for(Vertex* C1_v : C1_iter){
            G.new_timestamp();
            for(Vertex* x : C1_v->neighbors){
                x->marked = G.timestamp;
            }

            for(Vertex* Nc2 : C2->neighbors){
                if(Nc2->marked != G.timestamp){
                    assert(Nc2 != v);
                    assert(Nc2 != C1.first);
                    assert(Nc2 != C1.second);
                    assert(Nc2 != C2);
                    Edge* e = G.add_edge(Nc2, C1_v);
                    edges_added.push_back(e);
                }
            }
        }

        G.delete_vertex(C2);
        G.k--;
        G.sol_size++;
     }

     void undo(Graph* G) const{
        G->transform_access.pop_back();
        G->k++;
        G->sol_size--;

        G->restore_vertex(C2);

        for(auto it = edges_added.rbegin(); it != edges_added.rend(); it++){
            G->delete_edge(*it);
        }
        
        G->restore_vertex(v);
     }

     void resolve(Graph* G, bool partial){
        if(C1.first->data.resolved_status == UNKNOWN || C1.second->data.resolved_status == UNKNOWN){
            assert(v->data.resolved_status == UNKNOWN && C2->data.resolved_status == UNKNOWN);
            if(partial)
                return;
            assert(false);
        }

        if(C1.first->data.resolved_status == VC && C1.second->data.resolved_status == VC){
            C2->data.resolved_status = VC;
            v->data.resolved_status = EXCLUDED;
        }
        else{
            v->data.resolved_status = VC;
            C2->data.resolved_status = EXCLUDED;
            
            
        }

     }

     void debug_print(Graph* G, bool partial){
        cout << "#### DEG 3 CN\n";
        state vs = v->data.resolved_status;
        state c2s = C2->data.resolved_status;
        state C11s = C1.first->data.resolved_status;
        state C12s = C1.second->data.resolved_status;
        resolve(G, partial);
        state after_vs = v->data.resolved_status;
        state after_c2s = C2->data.resolved_status;
        state after_C11s = C1.first->data.resolved_status;
        state after_C12s = C1.second->data.resolved_status;

        G->print_op_line(v , "v" , vs, after_vs);
        G->print_op_line(C2, "C2", c2s, after_c2s);
        G->print_op_line(C1.first , "C1 - first" , C11s, after_C11s);
        G->print_op_line(C1.second , "C1 - second" , C12s, after_C12s);
        cout <<"#### DONE\n";
     }
};


bool deg3_rule(Graph& G){
    bool reduced = false;
    G.new_timestamp();
   
    while(G.max_degree >= 3 && !G.deg_lists[3].empty()){
        Vertex* v = G.deg_lists[3].back();
        assert(deg(v) == 3);
        Vertex* a = v->neighbors[0];
        Vertex* b = v->neighbors[1];
        Vertex* c = v->neighbors[2];

        const bool ab = a->adjacent_to(b);
        const bool bc = b->adjacent_to(c);
        const bool ca = c->adjacent_to(a);

        size_t edge_count = ab + bc + ca;

        if(edge_count == 0){
            G.history.emplace_back(new VC_deg3_IS(v, G));
        }
        else if(edge_count == 1){
            pair<Vertex*, Vertex*> C1;
            Vertex* C2;
            if(ab){
                C1 = {a, b};
                C2 = c;
            }else if(bc){
                C1 = {b, c};
                C2 = a;
            }else{
                C1 = {c, a};
                C2 = b;
            }
            G.history.emplace_back(new VC_deg3_CN(v, C1, C2, G));
        }else{
            bool dom = domination_rule_known(G, v);
            assert(dom);
            //domination_rule_known(G,v);
        }

        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;


}