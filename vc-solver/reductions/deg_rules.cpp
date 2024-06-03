#include "deg_rules.h"

void deg1_rule_single(Graph& G){
    Vertex* v = G.deg_lists[1].back();
    assert(v->degree() == 1);
    G.MM_select_neighbors(v);
        
}

bool deg1_rule(Graph& G){
    bool reduced = false;
    while(!G.deg_lists[1].empty()){
        deg1_rule_single(G);
        reduced = true;
    }
    return reduced;
}


class VC_deg2_fold : public VC_transformation{
public:
    Vertex* folded; // folded 
    Vertex* discard; // discarded
    Vertex* merged; // merged 

    vector<Edge*> moved_edges;
    
    // this pattern by aleksander of executing the rule while creating the operation is pretty neat I think
    VC_deg2_fold(Vertex* folded, Vertex* a, Vertex* b, Graph& G)
     : folded(folded), discard(a), merged(b) {
        G.delete_vertex(folded); //note: state stays UNKNOWN
        //a has smaller neighborhood
        if(discard->degree() > merged->degree())
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
        if(merged->status == INCLUDED){
            discard->status = INCLUDED;
            folded->status = EXCLUDED;
            return;
        }
        if(merged->status == EXCLUDED){
            discard->status = EXCLUDED;
            folded->status = INCLUDED;
            return;
        }

        assert(false && "partial solution not yet implemented"); //this shouldn't happen yet
    }
};

void deg2_rule_single(Graph& G){
    Vertex* v = G.deg_lists[2].back();
    assert(v->degree() == 2);

    Vertex* a = v->neighbors[0];
    Vertex* b = v->neighbors[1];

    assert(a->degree() > 0);
    assert(b->degree() > 0);

    if(a->adjacent_to(b)){
        //triangle case
        G.MM_select_neighbors(v);
    }
    else{
        G.history.emplace_back(new VC_deg2_fold(v, a, b, G));
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
    return reduced;
}

