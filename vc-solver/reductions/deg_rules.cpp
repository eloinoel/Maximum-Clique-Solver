#include "deg_rules.h"
#include "unconfined.h"

void deg0_rule(Graph& G){
    vector<Vertex*> deleted;
    while(!G.deg_lists[0].empty()){
        Vertex* v = G.deg_lists[0].back();
        deleted.push_back(v);
        G.deg_lists[0].pop_back();
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
    while(!G.deg_lists[1].empty()){
        deg1_rule_single(G);
        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;
}


class VC_deg2_fold : public VC_transformation{
public:
    Vertex* folded; // folded 
    Vertex* discard; // discarded
    Vertex* merged; // merged 

    vector<Edge*> moved_edges;
    
    // this pattern by aleksander of executing the rule while creating the operation is pretty neat I think
    VC_deg2_fold(Vertex* folded, Graph& G)
     : folded(folded) {
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
        G->sol_size--;
        G->k++;
        G->restore_vertex(discard);
        for(auto it = moved_edges.rbegin(); it != moved_edges.rend(); it++){
            G->delete_edge(*it);
        }
        G->restore_vertex(folded);
    }

    void resolve(Graph* G) {
        if(merged->status == VC){
            discard->status = VC;
            folded->status = EXCLUDED;
            return;
        }
        if(merged->status == EXCLUDED){
            discard->status = EXCLUDED;
            folded->status = VC;
            return;
        }

        assert(false && "partial solution not yet implemented"); //this shouldn't happen yet
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
    while(!G.deg_lists[2].empty()){
        deg2_rule_single(G);
        deg1_rule(G); //ensures min deg > 1, otherwise need to special case deg = 1 to avoid unnecessary folds
        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;
}

class VC_deg3_IS : public VC_transformation{
public: 
    Vertex *v;
    Vertex* IS[3]; // independset set = N(v)
    vector<Edge*> edges_added;
    Edge* connect_IS[2];

    VC_deg3_IS(Vertex* v, Graph& G)
     : v(v) {
        
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
        G->delete_edge(connect_IS[1]);
        G->delete_edge(connect_IS[0]);

        for(auto it = edges_added.rbegin(); it != edges_added.rend(); it++){
            G->delete_edge(*it);
        }

        G->restore_vertex(v);
     }

     void resolve(Graph* G){
        size_t count = 0;
        for(size_t i = 0; i < 3; i++){
            count += IS[i]->status == VC;
        }

        if(count == 3){
            assert(v->status == UNKNOWN);
            v->status = EXCLUDED;
        }
        else if(count == 2){
            v->status = VC;
            for(size_t i = 0; i < 3; i++){
                if(IS[i]->status == VC)
                    continue;

                IS[(i+1)%3]->status = EXCLUDED;
            }
        }else if(count == 0){
            assert(IS[1]->status == VC);
            IS[1]->status = EXCLUDED;
            v->status = VC;
        }else{
        assert(false); // not yet implemented partial solution
        }
     }
};


class VC_deg3_CN : public VC_transformation{
public:
    Vertex* v;
    pair<Vertex*, Vertex*> C1; // clique with 2 members
    Vertex* C2; // 'clique' with 1 member
    vector<Edge*> edges_added;

    VC_deg3_CN(Vertex* v_, pair<Vertex*, Vertex*> C1_,  Vertex* C2_, Graph& G)
     : v(v_), C1(C1_), C2(C2_){
        G.delete_vertex(v);
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
        G->k++;
        G->sol_size--;

        G->restore_vertex(C2);

        for(auto it = edges_added.rbegin(); it != edges_added.rend(); it++){
            G->delete_edge(*it);
        }
        
        G->restore_vertex(v);
     }

     void resolve(Graph* G){
        if(C1.first->status == VC && C1.second->status == VC){
            C2->status = VC;
        }
        else{
            v->status = VC;
        }
        //partial not yet implemented

     }
};


bool deg3_rule(Graph& G){
    bool reduced = false;
    G.new_timestamp();
   
    while(!G.deg_lists[3].empty()){
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
            //bool dom = domination_rule_known(G, v);
            //assert(dom);
            domination_rule_known(G,v);
        }

        reduced = true;
    }
    #if !AUTO_DEG0
    deg0_rule(G);
    #endif
    return reduced;


}