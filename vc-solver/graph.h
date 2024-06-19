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




enum state {UNKNOWN = -1, EXCLUDED, VC, CLIQUE, FOLDED};

#define DEBUG 0
#define USE_MIN_DEG 1
#define AUTO_DEG0 0

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
    int sol_size = 0;

    std::vector<Vertex*> partial; // the current solution state i.e current Clique
    //std::vector<Vertex*> best; // the best found solution so far

    int UB = numeric_limits<int>::max();
    int best = numeric_limits<int>::max();

    size_t max_degree = 0;

    #if USE_MIN_DEG
    size_t min_degree = int::max();
    #endif

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

    vector<string> name_table;

    #if DEBUG
    vector<size_t> deg_list_state;
    #endif

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

    Graph shallow_copy();

    /* add vertex at the end */
    Vertex* add_vertex();
    Vertex* add_vertex(size_t id);

    /* convenience functions to swap-delete */
    void pop_edge(size_t edge_idx);

    void pop_edge(Edge* e);

    void pop_vertex(Vertex* v);

    /* INTERNAL METHOD, only for Bruno */
    void restore_vertex(Vertex* v);

    /* INTERNAL METHOD, only for Bruno
     * use when vertex state stays UNKNOWN
     */
    void delete_vertex(Vertex* v);

    void delete_all();

    Graph complementary_graph(Graph& G);

    /**
     * Functions prefixed with MM_ are already memory managed
     * that is, the changes of the operations are saved in the history
     * and can be automatically reversed with undo()
     */

    /**
     * all of these assume that the vertex is not deg0 and do not check this
     * this includes the case you call discard twice, a neighbor may become deg0 and error
     * could change this/add a _SAFE variant when explicitely deleting multiple times without other checks in between
     */

    /* use to EXCLUDE vertex from solution */
    void MM_discard_vertex(Vertex* v);
    /* use to INCLUDE vertex to solution */
    void MM_vc_add_vertex(Vertex* v);
    /* EXCLUDE vertex from solution and INCLUDE N(v)*/
    void MM_vc_add_neighbors(Vertex* v);


    /** simple add without consistency check */
    void MM_clique_add_vertex(Vertex* v);

    /**
     * tries to add v to current clique C
     * returns true if v could be added (adjancent to all c in C), adds operation that can be undone
     * returns false otherwise, no operation or changes made
     *
     * CURRENTLY IT IS ASSUMED THE ORDER OF THE PARTIAL SOLUTION IS NOT MODIFIED,
     * IF YOU WANT THIS TO BE CHANGED, TELL ME - Bruno
     */
    bool MM_clique_add_vertex_if_valid(Vertex* candidate);

    void MM_induced_subgraph(vector<Vertex*>& induced_set);

    // CLOSED neighborhood N[v], if you want open neighborhood N(v) i.e not include v itself, just use MM_induced_subgraph(v->neighbors)
    void MM_induced_neighborhood(Vertex* v);

    Edge* add_edge(Vertex* u, Vertex* v);

    Edge* MM_add_edge(Vertex* u, Vertex* v);

    /* does not track any information for restore */
    void delete_edge(Edge* e);
    /**
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
     * when Vertex won't be in any list afterwards / want to use update_deglists on fresh list i.e. new subgraph/component
     */
    void delete_from_deglist(Vertex* v);

};

class Edge{

public:
    Endpoint ends[2];

    size_t idx; // index of Edge in E

    void flip(){
        swap(ends[0], ends[1]);
    }

};


class Vertex{
public:
    #if DEBUG
        string name;
    #endif

    size_t id;
    state status = UNKNOWN;
    unsigned long long marked = 0;

    size_t v_idx = -1; // because id won't match for components/induced subgraph and copying the vertex >could< lead to memory issues
    size_t deg_idx = -1; // index in degree list, needed to remove without search
    size_t list_idx = 0;


    std::vector<Vertex*> neighbors;
    std::vector<Edge*> edges;

    union data_union{
        struct{
            size_t clique_idx = 0;
            //size_t clique_size = 0;
        } clique_data;

        data_union(){};
        ~data_union(){};
    }data;

    /* convenience like above
     * requires the correct index of this vertex's end of the edge 
     */
    void pop_edge(size_t end_idx){
        assert(end_idx < edges.size());
        size_t side = (edges.back()->ends[0].v == this)? 0 : 1;
        edges.back()->ends[side].idx = end_idx;
        swap(edges[end_idx], edges.back());
        edges.pop_back();
    }

     void pop_neighbor(size_t n_idx){
        assert(n_idx < neighbors.size());
        swap(neighbors[n_idx], neighbors.back());
        neighbors.pop_back();
    }


    /* lazier version of the above, fine if the index is not known from context already */
    void pop_edge(Edge* e){
        assert(e->ends[0].v == this || e->ends[1].v == this);
        size_t side = (e->ends[0].v == this)? 0 : 1;
        edges.back()->ends[side].idx = e->ends[side].idx;
        swap(edges[e->ends[side].idx], edges.back());
        edges.pop_back();
    }

    void pop_neighbor(Edge* e){
        assert(e->ends[0].v == this || e->ends[1].v == this);
        size_t end_idx = (e->ends[0].v == this)? e->ends[0].idx : e->ends[1].idx;
        swap(neighbors[end_idx], neighbors.back());
        neighbors.pop_back();
    }

    /**
     * linear search through smaller neighborhood, sufficient for sparse graphs
     * however, since clique will usually deal with dense ones, maybe change?
     * potentially tree for log complexity?
     */
    bool adjacent_to(Vertex* u);

    // somewhat dangerous as size_t because of underflows, maybe turn into int
    [[nodiscard]] size_t inline degree() const
    {
        return neighbors.size();
    };

};

inline size_t deg(const Vertex* v){
    return v->neighbors.size();
}




