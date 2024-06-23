#include "AMTS.h"


#define not_in_S(x) x->data.tabu.s_idx == -1
#define in_S(x) x->data.tabu.s_idx != -1
#define TABU(x) (x->data.tabu.tabu_time >= iter)

#define LEGAL_CLIQUE ((N*(N-1))/2)

void AMTS::initalize_AMTS(Graph& G, int k_){

    k = k_;
    max_d = -1;
    min_s_d = numeric_limits<int>::max();
    C = max((int) k/40, 6);

    for(Vertex* v : G.V){
        v->data.tabu = { 0, // frequency 
                        -1, // s_idx
                        -1, // AB_idx
                         0, // tabutime
                         0};// dS
    }

    std::uniform_int_distribution<int> distr(0, G.deg_lists[G.max_degree].size() - 1);

    vector<Vertex*> init_sol(k);
    if(G.max_degree == 0)
        return;
    Vertex* s_prime = G.deg_lists[G.max_degree][distr(G.gen)];
    init_sol[0] = s_prime;
    s_prime->data.tabu.s_idx = 0;

    for(int c = 1; c < k; c++){
        auto s_time         = G.new_timestamp();
        Vertex* best        = nullptr;
        int best_edge_count = -1;
        for(int s_i = 0; s_i < c; s_i++){
            Vertex* s = init_sol[s_i];
            for(Vertex* n : s->neighbors){
                n->marked = max(n->marked + 1, s_time);
                if( (int) (n->marked - s_time) > best_edge_count){
                    best = n;
                    best_edge_count = n->marked - s_time;
                }
            }
        }
        if(best){
            init_sol[c] = best;
            best->data.tabu.s_idx = c;
        }else{
            //random vertex not in init_sol
            auto in_s = G.new_timestamp();
            for(Vertex* s : init_sol)
                s->marked = in_s;
            
            std::uniform_int_distribution<int> s_distr(0, G.N-1);

            while(true){
                int idx = s_distr(G.gen);
                if(G.V[idx]->marked != in_s){
                    init_sol[c] = G.V[idx];
                    G.V[idx]->data.tabu.s_idx = c;
                    break;
                }
            }
        }
        G.increase_timestamp(k+1);
    }

    S = init_sol;

}

void AMTS::frequency_init(Graph& G){
    Vertex* lowest_freq = nullptr;
    int min_freq = numeric_limits<int>::max();

    for(Vertex* v : G.V){
        v->data.tabu = { v->data.tabu.frequency, // frequency 
                        -1, // s_idx
                        -1, // AB_idx
                         0, // tabutime
                         0};// dS
    }



    vector<Vertex*> freq_init(k);

    int times_k = numeric_limits<int>::max();
    for(Vertex* v : G.V){
        times_k = min(v->data.tabu.frequency/k, times_k);
        if(times_k == 0)
            break;
    }
   
    for(Vertex* v : G.V){
        if(v->data.tabu.frequency - (times_k * k) < min_freq){
            min_freq = v->data.tabu.frequency - (times_k * k);
            lowest_freq = v; 
        }
    }

    freq_init[0] = lowest_freq;

    for(int c = 1; c < k; c++){
        auto f_time         = G.new_timestamp();
        Vertex* best        = nullptr;
        int best_edge_count = -1;
        int min_freq        = numeric_limits<int>::max();
        for(int s_i = 0; s_i < c; s_i++){
            Vertex* s = freq_init[s_i];
            for(Vertex* n : s->neighbors){
                n->marked = max(n->marked + 1, f_time);
                if( (int) (n->marked - f_time) > best_edge_count){
                    best = n;
                    best_edge_count = n->marked - f_time;
                    min_freq = n->data.tabu.frequency;
                }else if((int) (n->marked -f_time) == best_edge_count && n->data.tabu.frequency < min_freq){
                        best = n;
                        min_freq = n->data.tabu.frequency;
                }
            }
        }
        if(best){
            freq_init[c] = best;
            best->data.tabu.s_idx = c;
        }else{
            //random vertex not in init_sol
            auto in_s = G.new_timestamp();
            for(Vertex* s : freq_init)
                s->marked = in_s;
            
            std::uniform_int_distribution<int> s_distr(0, G.N-1);

            while(true){
                int idx = s_distr(G.gen);
                if(G.V[idx]->marked != in_s){
                    freq_init[c] = G.V[idx];
                    G.V[idx]->data.tabu.s_idx = c;
                    break;
                }
            }
        }
        G.increase_timestamp(k+1);
    }

    S = freq_init;
}

void AMTS::mark_tabu(Vertex* v, bool moved_from_S, Graph& G){
    uniform_int_distribution<int> tabu_random(0, C);
    int l1 = (N*(N-1))/2 - f_S();
    int l = min(10, l1);
    int random = tabu_random(G.gen);
    int time_in_tabu = (moved_from_S)? l + random : 0.6*l + 0.6*random;
    v->data.tabu.tabu_time = iter + time_in_tabu;
}

bool AMTS::tabu_search(Graph& G, int L){
    int I = 0;
    S_star = S;

    uniform_real_distribution<double> p(0, 1);
    const int l1 = (N*(N-1))/2 - f_S();
    const int l = min(10, l1);
    const double p_threshold = min(0.1, (double) (l+2)/N);

    while(I < L){
        Vertex * v, * u;
        auto [improving, moveT] = constrained_move(G);
        bool not_empty = moveT.first && moveT.second;
        if((improving || p(G.gen) >= p_threshold) && not_empty){
            // CN MOVE
            u = moveT.first;
            v = moveT.second;
        }else{
            auto move = probability_move(G);
            u = move.first;
            v = move.second;
        }
        //swap
        S[u->data.tabu.s_idx] = v;
        v->data.tabu.s_idx = u->data.tabu.s_idx;
        u->data.tabu.s_idx = -1;

        u->data.tabu.frequency++;
        v->data.tabu.frequency++;

        if(f_S() == LEGAL_CLIQUE){
            S_star = S;
            return true;
        }

        iter++;

        if(f_S() > f_S_star()){
            S_star = S;
            I = 0;
        }else{
            I++;
            mark_tabu(u, true, G);
            mark_tabu(v, false,  G);
        }

        update_AB(G, {u, v});
    }
    return false;
}

bool AMTS::execute(Graph& G, int k_, int iter_max){
    initalize_AMTS(G, k_);

    while(iter < iter_max){
        bool is_clique = tabu_search(G, N * k);
        if(is_clique)
            return true;
        else{
            frequency_init(G);
        }
    }
    return false;
}

void AMTS::create_AB(Graph& G){
    auto s_time = G.new_timestamp();

    // compute A and B
    for(Vertex* s : S){
        for(Vertex* n: s->neighbors){
            n->marked = max(n->marked + 1, s_time);
            const int d_n = (int) (n->marked - s_time);
            n->data.tabu.dS = d_n;
            if(!TABU(n)){
                if(not_in_S(n)){
                    if (d_n > max_d){
                        max_d = d_n;
                        clear_B();
                    }
                    if(d_n == max_d)
                        add_B(n);
                }else{
                    if(d_n < min_s_d){
                        min_s_d = d_n;
                        clear_A();
                    }
                    if(d_n == min_s_d)
                        add_A(n);
                }
            }
        }
    }
    assert(!A.empty());
    assert(!B.empty());
}

void AMTS::clear_A(){
    for(Vertex* a : A)
        a->data.tabu.AB_idx = -1;
    A.clear();
}

void AMTS::add_A(Vertex * a){
    a->data.tabu.AB_idx = A.size();
    A.push_back(a);
}

void AMTS::add_B(Vertex * b){
    b->data.tabu.AB_idx = B.size();
    B.push_back(b);
}

void AMTS::pop_A(Vertex* a){
    //if(a->data.tabu.AB_idx == -1)
    //    return;
    A.back()->data.tabu.AB_idx = a->data.tabu.AB_idx;
    swap(A[a->data.tabu.AB_idx], A.back());
    A.pop_back();
}

void AMTS::pop_B(Vertex* b){
    //if(b->data.tabu.AB_idx == -1)
    //    return;
    B.back()->data.tabu.AB_idx = b->data.tabu.AB_idx;
    swap(B[b->data.tabu.AB_idx], B.back());
    B.pop_back();
}


void AMTS::clear_B(){
    for(Vertex* b : B)
        b->data.tabu.AB_idx = -1;
    B.clear();
}

void AMTS::update_AB(Graph& G, pair<Vertex*, Vertex*> move){
    auto& [u, v] = move;

    int d_u = 0;
    // u is moved out of S
    /* update u neighborhood */
    for(Vertex* n : u->neighbors){
        d_u += in_S(n);
        n->data.tabu.dS--; // has one less neighbor in S;
        if(n->data.tabu.AB_idx != -1){
            if(in_S(n)){
                clear_A(); // new better A, n is only new member
                add_A(n);
            }
            else{
                pop_B(n); // no longer max
            }
        }
    }
    /* update for u */
    u->data.tabu.dS = d_u; // neighbors of u in S
    if(!TABU(u)){
        if(d_u > max_d){
            max_d = d_u;
            clear_B();
        }
        if(d_u == max_d){
            add_B(u);
        }
    }

    // v is moved into S
    int d_v = 0;
    /* update v neighborhood */
    for(Vertex* w : v->neighbors){
        d_v += in_S(w);
        w->data.tabu.dS++; // has one less neighbor in S;
        if(w->data.tabu.AB_idx != -1){
            if(in_S(w)){
                pop_A(w); // no longer minimum
            }
            else{
                clear_B(); // new best B, w only member
                add_B(w);
            }
        }
    }

    v->data.tabu.dS = d_v;
    if(!TABU(v)){
        if(d_v < min_s_d){
            min_s_d = d_u;
            clear_A();
        }
        if(d_u == min_s_d){
            add_A(u);
        }
    }

    // either A or B empty, adjust max/min deg accordingly and fill them again
    const bool fill_A = A.empty();
    const bool fill_B = B.empty();;

    if(fill_A || fill_B){
        vector<Vertex*> new_A;
        vector<Vertex*> new_B;
        int new_max_d = -1;
        int new_min_d = numeric_limits<int>::max();

        for(Vertex* v : G.V){
            if(!TABU(v)){
                if(in_S(v) && fill_A){
                    if(v->data.tabu.dS < new_min_d){
                        new_A.clear();
                        new_min_d = v->data.tabu.dS;
                    }
                    if(new_min_d == v->data.tabu.dS){
                        new_A.push_back(v);
                    }
                }
                else if(fill_B){
                    if(v->data.tabu.dS > new_max_d){
                        new_B.clear();
                        new_max_d = v->data.tabu.dS;
                    }
                    if(new_max_d == v->data.tabu.dS){
                        new_B.push_back(v);
                    }
                }
            }
        }
        if(fill_A){
            A = new_A;
            min_s_d = new_min_d;
            for(size_t i = 0; i < A.size(); i++)
                A[i]->data.tabu.AB_idx = i;
        }
        if(fill_B){
            B = new_B;
            max_d = new_max_d;
            for(size_t i = 0; i < B.size(); i++)
                B[i]->data.tabu.AB_idx = i;
        }

    }

    
}

pair<bool, pair<Vertex*, Vertex*>> AMTS::constrained_move(Graph& G){
    
    if(A.empty() || B.empty())
        return {false, {nullptr, nullptr}};

    vector<pair<Vertex*, Vertex*>> T_moves;

    for(Vertex* a : A){
        auto adj_a = G.new_timestamp();
        for(Vertex* n : a->neighbors){
            n->marked = adj_a;
        }

        for(Vertex* b : B){
            if(b->marked != adj_a){
                T_moves.push_back({a, b});
            }
        }
    }

    bool improving = true;
    
    if(T_moves.empty()){
        improving = ((max_d - min_s_d - 1) > 0);
    }else{
        improving = (max_d - min_s_d) > 0;
    }

    if(!T_moves.empty()){
        std::uniform_int_distribution<int> T_distr(0, T_moves.size()-1);
        return {improving, T_moves[T_distr(G.gen)]};
    }else{
        // random u from A and v from B
        std::uniform_int_distribution<int> A_distr(0, A.size()-1);
        std::uniform_int_distribution<int> B_distr(0, B.size()-1);

        Vertex* u = A[A_distr(G.gen)];
        Vertex* v = B[B_distr(G.gen)];
        return {improving, {u, v}};
    }
}

pair<Vertex*, Vertex*> AMTS::probability_move(Graph& G){
    std::uniform_int_distribution<int> S_distr(0, S.size()-1);
    Vertex* u = S[S_distr(G.gen)];

    for(Vertex* v : G.V){
        if(v->data.tabu.s_idx == -1 && v->data.tabu.dS <= floor(density * k)){
            return {u, v};
        }
    }
    assert(false);
    return {nullptr, nullptr};
}

