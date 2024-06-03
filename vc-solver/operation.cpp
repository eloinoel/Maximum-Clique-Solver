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

OP_RestorePoint::OP_RestorePoint() {}
void OP_RestorePoint::undo(Graph* G) const {}
