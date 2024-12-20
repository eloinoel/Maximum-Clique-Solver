#pragma once

#include "vector"
#include "graph.h"
#include <typeinfo>

class Graph;
class Vertex;
class Edge;

#define DEBUG_OPS 0

using namespace std;

/* basic operations, including simply removing/adding vertices/edges
 * don't impose unresolved future changes to the vertex cover
 */
class Operation {
public:
    virtual void undo(Graph* G) const = 0;
    virtual ~Operation() = default;
};

/* operations that require resolving the current graph state
 * to find the proper vertex cover of the original graph
 */
class VC_Transformation : public Operation{
public: 
    virtual void resolve(Graph* G, bool partial) = 0;
    #if DEBUG_OPS
    virtual void debug_print(Graph* G, bool partial) = 0;
    #endif
};

class OP_RestorePoint : public Operation {
    //maybe add debug info?
public:
    explicit OP_RestorePoint();
    void undo(Graph* G) const override;
};


class OP_AddVertex : public Operation {
private:
    Vertex* added;
public:
    explicit OP_AddVertex(Vertex* added);
    void undo(Graph* G) const override;
};

class OP_DiscardVertex : public Operation {
private:
    vector<Vertex*> discarded;
public:
    explicit OP_DiscardVertex(vector<Vertex*> discarded);
    explicit OP_DiscardVertex(Vertex* discarded);
    void undo(Graph* G) const override;
};

class OP_SelectVertex : public Operation {
private:
    std::vector<Vertex*> selected;
public:
    explicit OP_SelectVertex(vector<Vertex*> selected);
    explicit OP_SelectVertex(Vertex* selected);
    void undo(Graph* G) const override;
};

class OP_DeleteDeg0 : public Operation {
private:
    std::vector<Vertex*> deg0;
public:
    explicit OP_DeleteDeg0(vector<Vertex*> deg0);
    explicit OP_DeleteDeg0(Vertex* deg0);
    void undo(Graph* G) const override;
};

class OP_AddEdge : public Operation {
private:
    vector<Edge*> e_added;
public:
    explicit OP_AddEdge(vector<Edge*> e_added);
    explicit OP_AddEdge(Edge* e_added);
    void undo(Graph* G) const override;
};

class OP_AddClique : public Operation {
public:
    explicit OP_AddClique();
    void undo(Graph* G) const override;
};

class OP_InducedSubgraph : public Operation {
private:
    mutable vector<Vertex*> old_V;
    mutable vector<Edge*> old_E;
    mutable vector<vector<Vertex*>> old_list; // deg list before changing to induced subgraph
    mutable vector<Edge*> deleted_edges; // requires that edges have v in induced set as endpoint[0]!!!!
    mutable pair<int, int> degs;
public: 
    explicit OP_InducedSubgraph(vector<Vertex*> induced_set, Graph& G);
    void undo(Graph* G) const override;
};

class OP_SetPackingPoint : public Operation {
private:
    size_t packing_count;
public:
    explicit OP_SetPackingPoint(size_t point)
     : packing_count(point){}
    void undo(Graph* G) const override;
};

class OP_AddCli : public Operation {
private:
    Vertex* selected;
public: 
    explicit OP_AddCli(Vertex* selected_)
     : selected(selected_){}
    void undo(Graph* G) const override;
};

class OP_UpdatedMu : public Operation{
private:
    vector<pair<Vertex*, int>> old_mu_values;
public:
    explicit OP_UpdatedMu(vector<pair<Vertex*, int>>& old_)
     : old_mu_values(old_){}
    void undo(Graph* G) const override;
};


