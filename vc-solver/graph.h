#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <cassert>
#include <chrono>
#include <memory>
#include <random>
#include <sstream>

#include "operation.h"
#include "util.h"
#include "packing/packing.h"

using namespace std;


enum state {UNKNOWN = -1, EXCLUDED, VC, CLIQUE, FOLDED};

#define DEBUG 0
#define DEBUG_OPS 0
// 166212 - 15
// 1111   -  9
#define FIXED_RNG_SEED 0
#define USE_MIN_DEG 1
#define AUTO_DEG0 0
#define USE_PACK 1
#define USE_MIRROR 1
#define CLIQUE_SOLVER 1

constexpr bool UPDATE_G_EDGELIST = false;

class Vertex;
class Edge;
class Graph;
class Operation;
class VC_Transformation;
class Packing_Constraint;

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
    mt19937_64 gen;
    /** count num branches in search tree */
    unsigned long long num_branches = 0;
    /** used to mark vertices, eg. to find common neighbours */
    unsigned long long timestamp = 0;

    /* remaining budget for VC (used when searching for VC of size 1, 2, ...) */
    int k = 0;
    int sol_size = 0;

    std::vector<Vertex*> partial; // the current solution state i.e current Clique
    std::vector<Vertex*> best_sol; // the best found solution so far

    int UB = numeric_limits<int>::max();

    /* CLIQUE THING, basically same as UB but name is more fortunate there */
    int LB = 1;
    int old_LB = 0;

    int best = numeric_limits<int>::max();
    unsigned long long best_found = 0;
    
    size_t max_degree = 0;

    #if USE_MIN_DEG
    size_t min_degree = numeric_limits<int>::max();
    #endif

    /* vertices are never really deleted, only status updated*/
    vector<Vertex*> V;
    vector<Edge*> E;

    /* deg_list[0] = vertices with deg(v)=0, use @max_degree to get max_deg bucket
    * changes in graph change this list --> don't use for-loop to iterate
    * iterate with: while(!empty) { take front/back }
    */
    vector<vector<Vertex*>> deg_lists;


    vector<Packing_Constraint*> constraints;

    /* transformations need to be frequently resolved for packing
     * this allows accessing them without iterating the entire history
     * also avoids having to dynamic_cast<> which is fairly slow
     */
    vector<VC_Transformation*> transform_access;

     /**
     * used to easily undo graph changes
    */
    vector<Operation*> history;

    vector<string> name_table;

    #if DEBUG
    vector<pair<size_t, size_t>> deg_list_state;
    #endif

    // simple version at the moment, will need to be adjusting for vertex folding etc.
    void output_vc();

    void resolve_vc(bool partial);
    void set_current_vc();

    /* used for marking vertices */
    unsigned long long new_timestamp();
    void increase_timestamp(unsigned long long offset);

    /* set restore point in operation stack, eg. to restore to this point after branching*/
    void set_restore();
    /* undo all operations until restore point created with `set_restore` */
    void restore();
    /* undo last operation on stack */
    void undo();

    Graph shallow_copy();

    Graph complementary_graph(Graph& G);

    void delete_all();

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

    [[nodiscard]] Vertex* MM_add_vertex();
    /* use to EXCLUDE vertex from solution */
    void MM_discard_vertex(Vertex* v);
    /* use to INCLUDE vertex to solution */
    void MM_vc_add_vertex(Vertex* v);
    /* EXCLUDE vertex from solution and INCLUDE N(v)*/
    void MM_vc_add_neighbors(Vertex* v);

    /**
     * tries to add v to current clique C
     * returns true if v could be added (adjancent to all c in C), adds operation that can be undone
     * returns false otherwise, no operation or changes made
     * 
     * CURRENTLY IT IS ASSUMED THE ORDER OF THE PARTIAL SOLUTION IS NOT MODIFIED,
     * IF YOU WANT THIS TO BE CHANGED, TELL ME - Bruno
     */
    void MM_clique_add_vertex(Vertex* canditate); 

    bool MM_clique_add_vertex_if_valid(Vertex* candidate);

    // making this separate to avoid naming conflicts/confusion
    void MM_clisat_add_vertex(Vertex* canditate); 

    void MM_updated_mu(vector<pair<Vertex*, int>>& old_mu_values);

    void MM_induced_subgraph(vector<Vertex*>& induced_set);

    // CLOSED neighborhood N[v], if you want open neighborhood N(v) i.e not include v itself, just use MM_induced_subgraph(v->neighbors)
    void MM_induced_neighborhood(Vertex* v);

    Edge* add_edge(Vertex* u, Vertex* v);

    Edge* MM_add_edge(Vertex* u, Vertex* v);

    /* does not track any information for restore */
    void delete_edge(Edge* e);

    /* like delete_edge, but does NOT DELETE the edge, i.e. keeps in memory to readd later*/
    [[nodiscard]] Edge* remove_edge(Edge* e);
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

    string enum_name(state s){
        switch(s){
            case UNKNOWN:
                return "UNKNOWN";
            case EXCLUDED:
                return "EXCLUDED";
            case VC:
                return "VC";
            case CLIQUE:
                return "CLIQUE";
            default:
                return "ERROR STATE";
        }
    }

    void print_op_line(Vertex* v, string name, state before, state after);

    Graph(){ 
        #if FIXED_RNG_SEED
            cout << "------------FIXED RNG SEED TO " << FIXED_RNG_SEED << "------------\n";
            gen.seed(FIXED_RNG_SEED);
        #else
        random_device rd;
        gen.seed(rd());
        #endif
    }
};

class Edge{

public:
    Endpoint ends[2];

    size_t idx; // index of Edge in E 

    #if DEBUG
    size_t flip_count = 0;
    #endif

    void flip(){
        swap(ends[0], ends[1]);
        #if DEBUG
        flip_count++;
        #endif
    }
    
};


class Vertex{
public:
    #if !NDEBUG || DEBUG
        string name;
    #endif

    size_t id;
    state status = UNKNOWN; 
    unsigned long long marked = 0;

    size_t v_idx = -1; // because id won't match for components/induced subgraph and copying the vertex >could< lead to memory issues
    size_t deg_idx = -1; // index in degree list, needed to remove without search
    size_t list_idx = -1;
    

    std::vector<Vertex*> neighbors; 
    std::vector<Edge*> edges;

    // messy, should divide up clique solver and vc properly at some point
    #if CLIQUE_SOLVER
    int order_pos = -1;
    int mu = 0;
    #endif

    #if DEBUG
    void check_edge_consistency(){
      for(Edge* e : edges){
        assert(e->ends[0].v == this || e->ends[1].v == this);
        assert(e->ends[0].idx < (e->ends[0].v)->neighbors.size() && e->ends[1].idx < (e->ends[1].v)->neighbors.size());
        bool consistent = false;
        Vertex* expected[2] = {e->ends[0].v, e->ends[1].v};
        Vertex* found[2];
        for (size_t i = 0; i < 2; i++){
            found[i] = e->ends[i].v->neighbors[e->ends[i].idx];
            consistent |= found[i] == this;
        }
        assert(consistent);
      }  
    } 
    #endif

    union data_union{
        struct{
            size_t clique_idx = 0;
            //size_t clique_size = 0;
        } clique_data;

        struct{
            Vertex* next = nullptr;
            size_t clique_size = 0;
            int clique_idx = 0;
        } block;

        struct{
            bool Amarked;
            bool Bmarked;
        } desk;

        struct{
            int count;
        }packing;


        /* clique stuff */
        struct{
        int iclass;
        int reference_class;
        int filter_status;
        } iset;

        state resolved_status;

        Vertex* found;

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
    [[nodiscard]] size_t inline degree() const;

};

inline size_t deg(const Vertex* v){
    return v->neighbors.size();
}




