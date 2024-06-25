#pragma once
#include "graph.h"
#include "chrono"

using ts = chrono::high_resolution_clock::time_point;

class AMTS{
public:
    double density;
    int k = -1;
    int max_d = -1;
    int min_s_d = numeric_limits<int>::max();

    int N;

    int iter = 0;
    int C = 6;

    vector<Vertex*> S;
    int f_star = -1;
    vector<Vertex*> S_star;

    vector<Vertex*> A;
    vector<Vertex*> B;
public:
    void initalize_AMTS(Graph& G, int k_);
    void frequency_init(Graph& G);
    bool execute(Graph& G, int k_, int iter_max);
    bool execute_timed(Graph& G, int k_, ts start, int max_ms);
    void create_AB(Graph& G);
    void update_AB(Graph& G, pair<Vertex*, Vertex*> move);
    bool tabu_search(Graph& G, int L);
    bool tabu_search_timed(Graph& G, int Lm, ts start, int max_ms);
    pair<bool, pair<Vertex*, Vertex*>> constrained_move(Graph& G);
    pair<Vertex*, Vertex*> probability_move(Graph& G);

    void mark_tabu(Vertex* v, bool moved_to_S, Graph& G);

    int f_S(){
        int sum = 0;
        for(Vertex* s : S){
            sum += s->data.tabu.dS;
        }
        return sum/2;
    }

    int f_S_star(){
        int sum = 0;
        for(Vertex* s : S_star){
            sum += s->data.tabu.dS;
        }
        return sum/2;
    }

    void clear_A();
    void clear_B();
    void add_A(Vertex* a);
    void add_B(Vertex* b);
    void pop_A(Vertex* a);
    void pop_B(Vertex* b);


    AMTS(Graph& G){
        N = G.N;
        if(G.V.size() < 2)
            density = 1.0;
        else{
            const int M = G.E.size();
            density =  (double) (2*M)/(N * (N-1));
        }
    }
};