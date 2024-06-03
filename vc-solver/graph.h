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




enum states {UNKNOWN = -1, EXCLUDED, INCLUDED, FOLDED};

#define DEBUG 0
constexpr bool UPDATE_G_EDGELIST = false;

class Vertex;
class Edge;
class Graph;
class Operation;


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
    unsigned long long num_branches = 0;
    unsigned long long timestamp = 0;


    int k = 0;
    int sol_size = 0;

    size_t max_degree = 0;

    vector<Vertex*> V;
    vector<Edge*> E;

    vector<vector<Vertex*>> deg_lists;

    vector<Operation*> history;

    vector<string> name_table;

    // simple version at the moment, will need to be adjusting for vertex folding etc.
    void output_vc();

    void new_timestamp();
    void increase_timestamp(unsigned long long offset);

    void undo();
    void restore();
    void set_restore();

    Vertex* add_vertex();
    Vertex* add_vertex(size_t id);

    void restore_vertex(Vertex* v);

    // use when vertex state stays UNKNOWN
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
    // use when vertex state is EXCLUDED
    void MM_discard_vertex(Vertex* v);
    // use when vertex state is INCLUDED
    void MM_select_vertex(Vertex* v);
    // use when vertex state is EXCLUDED and N(v) INCLUDED
    void MM_select_neighbors(Vertex* v);

    Edge* add_edge(Vertex* u, Vertex* v);

    // does not track any information for restore
    void delete_edge(Edge* e);
    /**
     * keeps information for easy restore
     * use for simple case of deleting a vertex that will be restored to exact same state
     */
    void delete_edge_lazy(Edge* e, Vertex* v);


    void update_deglists(Vertex* v);
    // for moving between deglists
    void remove_from_deglist(Vertex* v);
    // when Vertex won't be in any list afterwards
    void delete_from_deglist(Vertex* v);

};


class Vertex{
public:
    #if DEBUG
        string name;
    #endif

    size_t id;
    int8_t status = UNKNOWN; // -1 = unknown, 0 = deleted, 1 = solution, 2 = folded
    unsigned long long marked = 0;

    size_t deg_idx = -1; // index in degree list, needed to remove without search
    size_t list_idx = 0;
    bool deleted = false;

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

    size_t idx;
    
};



