#pragma once

#include "vector"
#include "graph.h"
#include <typeinfo>

class Graph;
class Vertex;

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
class VC_transformation : public Operation{
public: 
    virtual void resolve(Graph* G) = 0;
};

class OP_RestorePoint : public Operation {
    //maybe add debug info?
public:
    explicit OP_RestorePoint();
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

