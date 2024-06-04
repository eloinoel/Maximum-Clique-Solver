#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <cassert>
#include <chrono>
#include <memory>

#include "operation.h"
#include "util.h"

using namespace std;

/**
 * UKNOWN: don't know whether is in VC, can be in graph or not
 * EXCLUDED: can't be in VC, not in graph
 * INCLUDED: is in VC, not in graph
 * FOLDED: merged with other vertex, not in graph, not relevant for MC
*/
enum state {UNKNOWN = -1, EXCLUDED, INCLUDED, FOLDED};

#define DEBUG 0

/** not relevant for MC */
constexpr bool UPDATE_G_EDGELIST = false;

class Vertex;
class Edge;
class Graph;
class Operation;

/** Endpoints of Edges */
typedef struct{
        Vertex* v = nullptr;
        size_t idx = 0;
} Endpoint;

class Graph {
public:
    size_t total_N = 0;   //total N, M of initial instance
    size_t total_M = 0;

    size_t N = 0;   //remaining N, M
    size_t M = 0;

    //Counter c;// mostly debug helper, could be extended to collect profiling information
    Timer timer;
    /** count num branches in search tree */
    unsigned long long num_branches = 0;
    /** used to mark vertices, eg. to find common neighbours */
    unsigned long long timestamp = 0;

    /* remaining budget for VC (used when searching for VC of size 1, 2, ...) */
    int k = 0;
    /* current solution size */
    int sol_size = 0;

    /* max degree in current graph */
    size_t max_degree = 0;

    /* vertices are never really deleted, only status update*/
    vector<Vertex*> V;
    vector<Edge*> E;

    /* deg_list[0] = vertices with deg(v)=0, use @max_degree to get max_deg bucket
    * changes in graph change this list --> don't use for-loop to iterate
    * iterate with: while(!empty) { take front/back }
    */
    vector<vector<Vertex*>> deg_lists;

    /**
     * used to easily undo graph changes
    */
    vector<Operation*> history;

    /* map vertex index to string name */
    vector<string> name_table;

    // simple version at the moment, will need to be adjusting for vertex folding etc.
    void output_vc();

    /* used for marking vertices */
    void new_timestamp();
    void increase_timestamp(unsigned long long offset);

    /* set restore point in operation stack, eg. to restore to this point after branching*/
    void set_restore();
    /* undo all operations until restore point created with `set_restore` */
    void restore();
    /* undo last operation on stack */
    void undo();


    /* add vertex at the end */
    Vertex* add_vertex(); // used to construct graph
    Vertex* add_vertex(size_t id); // only Bruno uses this

    /* INTERNAL METHOD, only for Bruno
    */
    void restore_vertex(Vertex* v);

    /* INTERNAL METHOD, only for Bruno
    * use when vertex state stays UNKNOWN
    */
    void delete_vertex(Vertex* v);

    /**---------------------------------------------------------------------
     * Functions prefixed with MM_ are already memory managed
     * that is, the changes of the operations are saved in the history
     * and can be automatically reversed with undo()
     */

    /* use to EXCLUDE vertex from solution */
    void MM_discard_vertex(Vertex* v);
    /* use to INCLUDE vertex to solution */
    void MM_select_vertex(Vertex* v);
    /* EXCLUDE vertex from solution and INCLUDE N(v)*/
    void MM_select_neighbors(Vertex* v);
    /**
     * all of these assume that the vertex is not deg0 and do not check this
     * this includes the case you call discard twice, a neighbor may become deg0 and error
     * could change this/add a _SAFE variant when explicitely deleting multiple times without other checks in between
     */
    // ---------------------------------------------------------------------

    /*  */
    Edge* add_edge(Vertex* u, Vertex* v);

    /*
    * does not track any information for restore
    */
    void delete_edge(Edge* e);
    /** INTERNAL, is used by `delete_edge`
     * keeps information for easy restore
     * use for simple case of deleting a vertex that will be restored to exact same state
     */
    void delete_edge_lazy(Edge* e, Vertex* v);

    /* INTERNAL */
    void update_deglists(Vertex* v);
    /* INTERNAL
     * for moving between deglists
     */
    void remove_from_deglist(Vertex* v);
    /* INTERNAL
     * when Vertex won't be in any list afterwards
     */
    void delete_from_deglist(Vertex* v);

};


class Vertex{
public:
    #if DEBUG
        string name;
    #endif

    size_t id;
    state status = UNKNOWN;
    unsigned long long marked = 0;

    size_t deg_idx = -1; // index in degree list, needed to remove without search
    size_t list_idx = 0;
    bool deleted = false; // check status instead, Bruno is not sure if this will be needed

    std::vector<Vertex*> neighbors;
    std::vector<Edge*> edges;

    union data_union {
        struct{
            size_t clique_idx = 0;
            //size_t clique_size = 0;
        } clique_data;

        data_union(){};
        ~data_union(){};
    } data;

    /**
     * linear search through smaller neighborhood, sufficient for sparse graphs
     * however, since clique will usually deal with dense ones, maybe change?
     * potentially tree for log complexity?
     */
    bool adjacent_to(Vertex* u);

    // somewhat dangerous as size_t because of underflows, maybe turn into int
    [[nodiscard]] size_t degree() const;

};

class Edge{

public:
    Endpoint ends[2];

    size_t idx; // Edge itself has an index
};



