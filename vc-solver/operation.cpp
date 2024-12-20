#include "operation.h"

OP_DiscardVertex::OP_DiscardVertex(vector<Vertex*> discarded)
 : discarded(discarded) {}

OP_DiscardVertex::OP_DiscardVertex(Vertex* discarded)
 : discarded({discarded}) {}

void OP_DiscardVertex::undo(Graph* G) const {
    for(auto& v : discarded)
        G->restore_vertex(v);
}

OP_SelectVertex::OP_SelectVertex(vector<Vertex*> selected)
 : selected(selected) {}

OP_SelectVertex::OP_SelectVertex(Vertex* selected)
 : selected({selected}) {}

void OP_SelectVertex::undo(Graph* G) const {
    for(auto& v : selected){
        G->restore_vertex(v);
        G->k++;
        G->sol_size--;
    }
}

OP_AddVertex::OP_AddVertex(Vertex* added)
 : added(added) {}

 void OP_AddVertex::undo(Graph* G) const {
    G->pop_vertex(added);
    if(added->list_idx != (size_t) - 1)
        G->delete_from_deglist(added);
    G->N--;
    delete added;
 }

OP_DeleteDeg0::OP_DeleteDeg0(vector<Vertex*> deg0)
 : deg0(deg0) {}

OP_DeleteDeg0::OP_DeleteDeg0(Vertex* deg0)
 : deg0({deg0}) {}

void OP_DeleteDeg0::undo(Graph* G) const {
    for(auto& v : deg0){
        v->status = UNKNOWN;
        G->N++;
        G->update_deglists(v);
    }
}

OP_AddEdge::OP_AddEdge(vector<Edge*> e_added)
 : e_added(e_added) {}

OP_AddEdge::OP_AddEdge(Edge* e_added)
 : e_added({e_added}) {}

void OP_AddEdge::undo(Graph* G) const {
    for(auto it = e_added.rbegin(); it != e_added.rend(); it++){
        G->delete_edge(*it);
    }
}

OP_AddClique::OP_AddClique() {}

void OP_AddClique::undo(Graph* G) const {
    G->k++;
    G->sol_size--;
    Vertex* c = G->partial.back();
    c->status = UNKNOWN;
    G->partial.pop_back();
}

OP_InducedSubgraph::OP_InducedSubgraph(vector<Vertex*> induced_set, Graph& G)
{
    G.new_timestamp();

    degs = {G.max_degree, G.min_degree};

    for(Vertex* v : induced_set)
        v->marked = G.timestamp;
 
    size_t i = 0;
    for(Vertex* v : induced_set){
        G.pop_vertex(v);
        G.delete_from_deglist(v);
        size_t j    =  0;
        v->v_idx    =  i++;
        while(j < v->neighbors.size()){
            Vertex* n = v->neighbors[j];
            Edge*   e = v->edges[j];
    
            // either keep edge and update its idx or delete it
            if(n->marked == G.timestamp){
                if(n < v){
                    G.pop_edge(e);
                    e->idx        = old_E.size();
                    old_E.push_back(e); // old_E will be the new E after swap
                }
                size_t side       = (e->ends[0].v == v)? 0 : 1;
                e->ends[side].idx = j++;
            }
            else{
                if(e->ends[0].v != v){ // important to keep this consistent! As the edge is not gonna be in the graph following this operation, unless you do weird things, this shouldn't be a problem....
                    e->flip();
                }
               
                v->pop_edge(j);
                v->pop_neighbor(j);
                G. pop_edge(e);

                deleted_edges.push_back(e);
            }
        }
    }

   
    swap(G.deg_lists, old_list);

    G.max_degree = 0;
    G.min_degree = numeric_limits<int>::max();

    for(Vertex* s : induced_set){
        G.update_deglists(s);
    }

    old_V = move(induced_set);
  
    
    swap(G.V, old_V);
    swap(G.E, old_E);
    
    
    G.N = G.V.size();
    G.M = G.E.size();

 }

void OP_InducedSubgraph::undo(Graph* G) const {
    //at the point where this is called, only the induced edges should remain
    for(Edge* e : deleted_edges){
        Vertex* v               = e->ends[0].v;
        e->ends[0].idx          = v->edges.size();
        e->idx                  = old_E.size();
        old_E.push_back(e);
        v->edges.push_back(e);
        v->neighbors.push_back(e->ends[1].v);
    }

    swap(G->deg_lists, old_list);

    for(Vertex* v : G->V){
        v->v_idx = old_V.size();
        old_V.push_back(v);
        v->deg_idx = -1;
        v->list_idx = -1;
        G->update_deglists(v);
    }

    for(Edge* e : G->E){
        e->idx = old_E.size();
        old_E.push_back(e);
    }

    swap(G->E, old_E);
    swap(G->V, old_V);

    G->M = G->E.size();
    G->N = G->V.size();

    G->max_degree = degs.first;
    G->min_degree = degs.second;
}

void OP_SetPackingPoint::undo(Graph* G) const {
    auto p = G->constraints.size();
    assert(packing_count <= G->constraints.size());
    while(G->constraints.size() > packing_count){
        delete G->constraints.back();
        G->constraints.pop_back();
    }
}

OP_RestorePoint::OP_RestorePoint() {}
void OP_RestorePoint::undo(Graph* G) const {}

void OP_AddCli::undo(Graph* G) const{
    selected->status = UNKNOWN;
    G->partial.pop_back();
}

void OP_UpdatedMu::undo(Graph* G) const {
    for(auto& [v, mu] : old_mu_values)
        v->mu = mu;
}
