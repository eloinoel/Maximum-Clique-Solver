#include "AMTS.h"


#define not_in_S(x) (x->data.tabu.s_idx == -1)
#define in_S(x) (x->data.tabu.s_idx != -1)
#define TABU(x) (x->data.tabu.tabu_time > iter)
#define degS(x) (x->data.tabu.dS)

#define OUT_OF_TIME (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() > max_ms)

#define LEGAL_CLIQUE ((k*(k-1))/2)


vector<string> AMTS::find_best(int LB, int max_ms, Graph& G){
    vector<string> clique_found;

    ts start = chrono::high_resolution_clock::now();
  
    for(int _k = LB; _k <= G.N; _k++){
       
        initalize(G, _k);
        if(execute_timed(G, _k, start, max_ms)){
            
            clique_found.clear();
            for(Vertex* x : S_star){
                clique_found.push_back(G.name_table[x->id]);
            }
        }else{
            //cout << "amts did not find " << _k << "\n";
            break;
        }
    }
    return clique_found;
 }

void AMTS::initalize(Graph& G, int k_){

    iter            =  0;
    k               =  k_;
    B_deg           = -1;
    candidate_B_deg = -1;
    A_deg           =  numeric_limits<int>::max();
    candidate_A_deg =  numeric_limits<int>::max();

    C = max((int) k/40, 6);

    for(Vertex* v : G.V){
        v->data.tabu = {0};
        v->data.tabu.frequency  =  0;
        v->data.tabu.s_idx      = -1;
        v->data.tabu.AB_idx     = -1;
        degS(v)                 =  0;
        v->data.tabu.tabu_time  =  0;
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
                if(n->data.tabu.s_idx == -1){
                    n->marked = max(n->marked + 1, s_time);
                    if( (int) (n->marked - s_time) > best_edge_count){
                        best = n;
                        best_edge_count = n->marked - s_time;
                    }
                }
            }
        }
        if(best){
            init_sol[c] = best;
            best->data.tabu.s_idx = c;
        }else{
            std::uniform_int_distribution<int> s_distr(0, G.N-1);

            while(true){
                int idx = s_distr(G.gen);
                if(G.V[idx]->data.tabu.s_idx == -1){
                    init_sol[c] = G.V[idx];
                    G.V[idx]->data.tabu.s_idx = c;
                    break;
                }
            }
        }
        G.increase_timestamp(k+1);
    }

    S = init_sol;
    for(Vertex* s : S)
        assert(in_S(s));

}

void AMTS::frequency_init(Graph& G){
    for(Vertex* v : G.V){
        v->data.tabu = { v->data.tabu.frequency, // frequency 
                        -1, // s_idx
                        -1, // AB_idx
                         0, // tabutime
                         0};// dS
    }

    //for(Vertex* s : S)
    //    s->data.tabu.s_idx = -1;


    vector<Vertex*> freq_init(k);

    int times_k = numeric_limits<int>::max();
    for(Vertex* v : G.V){
        times_k = min(v->data.tabu.frequency/k, times_k);
        if(times_k == 0)
            break;
    }

    if(times_k > 0){
        for(Vertex* v : G.V){
            v->data.tabu.frequency -= k * times_k;
        }
    }

    Vertex* lowest_freq = nullptr;
    int min_freq = numeric_limits<int>::max();
   
    for(Vertex* v : G.V){
        if(v->data.tabu.frequency < min_freq){
            min_freq = v->data.tabu.frequency;
            lowest_freq = v; 
        }
    }

    freq_init[0] = lowest_freq;
    lowest_freq->data.tabu.s_idx = 0;

    for(int c = 1; c < k; c++){
        auto f_time         = G.new_timestamp();
        Vertex* best        = nullptr;
        int best_edge_count = -1;
        int min_freq        = numeric_limits<int>::max();
        for(int s_i = 0; s_i < c; s_i++){
            Vertex* s = freq_init[s_i];
            for(Vertex* n : s->neighbors){
                if(n->data.tabu.s_idx == -1){
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
        }
        if(best){
            freq_init[c] = best;
            best->data.tabu.s_idx = c;
        }else{
            //random vertex not in init_sol
            std::uniform_int_distribution<int> s_distr(0, G.N-1);

            while(true){
                int idx = s_distr(G.gen);
               if(G.V[idx]->data.tabu.s_idx == -1){
                    freq_init[c] = G.V[idx];
                    G.V[idx]->data.tabu.s_idx = c;
                    break;
                }
            }
        }
        G.increase_timestamp(k+1);
    }

    S = freq_init;
    for(Vertex* s : S)
        assert(in_S(s));
    clear_A();
    clear_B();
    clear_Set(A_candidate);
    clear_Set(B_candidate);
    create_AB(G);
}

void AMTS::mark_tabu(Vertex* v, bool moved_from_S, Graph& G){
    uniform_int_distribution<int> tabu_random(0, C);
    long long unsigned l1 = (N*(N-1))/2 - f_S();
    long long unsigned l  = min((long long unsigned) 10, l1);
    int random = tabu_random(G.gen);
    int time_in_tabu = (moved_from_S)? ((int) l)  + random : 0.6* ((int) l) + 0.6*random;
    //cout << "tabu timer" << time_in_tabu << "\n";
    v->data.tabu.tabu_time = iter + time_in_tabu;

    if(v->data.tabu.AB_idx != -1){
        if(in_S(v)){
            pop_A(v);
        }
        else{
            pop_B(v);
        }
    }

}

bool AMTS::tabu_search(Graph& G, int L){
    int I = 0;
    
    uniform_real_distribution<double> p(0, 1);

    while(I < L){
        for(Vertex* s : S)
            assert(in_S(s));
        for(Vertex* a : A)
            assert(in_S(a));
        for(Vertex* b : B)
            assert(not_in_S(b));

        Vertex * w = nullptr;
        Vertex * u = nullptr;
        const int l1 = (N*(N-1))/2 - f_S();
        const double p_threshold = min(0.1, (double) (l1+2)/N);

        auto [improving, moveT] = constrained_move(G);
        bool not_empty = moveT.first && moveT.second;
        if((improving || p(G.gen) >= p_threshold) && not_empty){
            // CN MOVE
            //cout << "took cn move\n";
            u = moveT.first;
            w = moveT.second;
            //cout << "took this\n";
        }else{
            pair<Vertex*, Vertex*> move = probability_move(G);
            u = move.first;
            w = move.second;
            assert(w->data.tabu.s_idx == -1);
        }

        assert(u->data.tabu.s_idx != -1);
        assert(w->data.tabu.s_idx == -1);

        //cout << "iter: " << iter << " [" << u->id << ", " << u->data.tabu.tabu_time << "] | " <<  "[" << w->id << ", " << w->data.tabu.tabu_time << "]" << "\n";
        //cout << boolalpha << "u tabu = " << TABU(u) << "\n";

        mark_tabu(u, true, G);
        mark_tabu(w, false,  G);

        // swap
        S[u->data.tabu.s_idx] = w;
        w->data.tabu.s_idx = u->data.tabu.s_idx;
        assert(u->data.tabu.s_idx != -1);
        u->data.tabu.s_idx = -1;

        for(Vertex* s : S)
            assert(in_S(s));

        u->data.tabu.frequency++;
        w->data.tabu.frequency++;

        update_AB(G, {u, w});
        cout << "NEEDED " << LEGAL_CLIQUE << ", CURRENT " << f_S() << ", STAR " << f_star << "\n";

        if(f_S() == LEGAL_CLIQUE){
            S_star = S;
            return true;
        }

        iter++;

        if(f_S() > f_star){
            //cout << "does this work?\n";
            //cout << "NEEDED " << LEGAL_CLIQUE << ", CURRENT " << f_S() << ", STAR " << f_star << "\n";
            S_star = S;
            f_star = f_S();
            I = 0;
            /* improving move - untabu */
            u->data.tabu.tabu_time = iter;
            w->data.tabu.tabu_time = iter;
        }else{
            I++;
        }

        if(!TABU(u)){
            if(u->data.tabu.dS > B_deg){
                B_deg = u->data.tabu.dS;
                clear_B();
            }
            if(u->data.tabu.dS == B_deg){
                assert(u->data.tabu.AB_idx == -1);
                add_B(u);
            }
        }

        if(!TABU(w)){
            if(w->data.tabu.dS < A_deg){
                A_deg = w->data.tabu.dS ;
                clear_A();
            }
            if(w->data.tabu.dS  == A_deg){
                assert(w->data.tabu.AB_idx == -1);
                add_A(w);
        }
    }
        
    }
    return false;
}

bool AMTS::tabu_search_timed(Graph& G, int L, ts start, int max_ms){
    int I = 0;
    
    uniform_real_distribution<double> p(0, 1);

    while(I < L){

        if(OUT_OF_TIME)
            return false;

        for(Vertex* s : S)
            assert(in_S(s));
        for(Vertex* a : A)
            assert(in_S(a));
        for(Vertex* b : B)
            assert(not_in_S(b));

        Vertex * w = nullptr;
        Vertex * u = nullptr;
        const int l1 = (N*(N-1))/2 - f_S();
        const double p_threshold = min(0.1, (double) (l1+2)/N);

        auto [improving, moveT] = constrained_move(G);
        bool not_empty = moveT.first && moveT.second;
        if((improving || p(G.gen) >= p_threshold) && not_empty){
            // CN MOVE
            //cout << "took cn move\n";
            u = moveT.first;
            w = moveT.second;
            //cout << "took this\n";
        }else{
            pair<Vertex*, Vertex*> move = probability_move(G);
            u = move.first;
            w = move.second;
            assert(w->data.tabu.s_idx == -1);
        }

        assert(u->data.tabu.s_idx != -1);
        assert(w->data.tabu.s_idx == -1);

        //cout << "iter: " << iter << " [" << u->id << ", " << u->data.tabu.tabu_time << "] | " <<  "[" << w->id << ", " << w->data.tabu.tabu_time << "]" << "\n";
        //cout << boolalpha << "u tabu = " << TABU(u) << "\n";

        mark_tabu(u, true, G);
        mark_tabu(w, false,  G);

        // swap
        S[u->data.tabu.s_idx] = w;
        w->data.tabu.s_idx = u->data.tabu.s_idx;
        assert(u->data.tabu.s_idx != -1);
        u->data.tabu.s_idx = -1;

        for(Vertex* s : S)
            assert(in_S(s));

        u->data.tabu.frequency++;
        w->data.tabu.frequency++;

        update_AB(G, {u, w});
        //cout << "NEEDED " << LEGAL_CLIQUE << ", CURRENT " << f_S() << ", STAR " << f_S_star() << "\n";

        if(f_S() == LEGAL_CLIQUE){
            S_star = S;
            return true;
        }

        iter++;

        if(f_S() > f_star){
            //cout << "does this work?\n";
            //cout << "NEEDED " << LEGAL_CLIQUE << ", CURRENT " << f_S() << ", STAR " << f_star << "\n";
            S_star = S;
            f_star = f_S();
            I = 0;
            /* improving move - untabu */
            u->data.tabu.tabu_time = iter;
            w->data.tabu.tabu_time = iter;
        }else{
            I++;
        }

        if(!TABU(u)){
            if(u->data.tabu.dS > B_deg){
                B_deg = u->data.tabu.dS;
                clear_B();
            }
            if(u->data.tabu.dS == B_deg){
                add_B(u);
            }
        }

        if(!TABU(w)){
            if(w->data.tabu.dS < A_deg){
                A_deg = w->data.tabu.dS ;
                clear_A();
            }
            if(w->data.tabu.dS  == A_deg){
                add_A(w);
        }

        //cout << "time = " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() << "\n";
    }
        
    }
    return false;
}

bool AMTS::execute(Graph& G, int k_, int iter_max){
    initalize(G, k_);
    create_AB(G);


    for(Vertex* a : A)
        assert(in_S(a));
    for(Vertex* b : B)
        assert(not_in_S(b));

    S_star = S;
    f_star = f_S();

    if(f_S() == LEGAL_CLIQUE){
        return true;
    }

    while(iter < iter_max){
        bool is_clique = tabu_search(G, N * k);
        if(is_clique)
            return true;
        else{
            frequency_init(G);
        }
        //cout << iter << "\n";
    }
    return false;
}

bool AMTS::execute_timed(Graph& G, int k_, ts start, int max_ms ){
    initalize(G, k_);
    create_AB(G);

    for(Vertex* a : A)
        assert(in_S(a));
    for(Vertex* b : B)
        assert(not_in_S(b));

    S_star = S;
    f_star = f_S();
    
    /* catches the case that all of G is a clique, i.e B is empty */
    if(f_S() == LEGAL_CLIQUE){
        return true;
    }

    while(!OUT_OF_TIME){
        bool is_clique = tabu_search_timed(G, N * k, start, max_ms);
        if(is_clique)
            return true;
        else{
            if(OUT_OF_TIME)
                return false;
            frequency_init(G);
        }
        //cout << iter << "\n";
    }
    return false;
}

void AMTS::update_A(Vertex* a){
    const int d_a = a->data.tabu.dS;
    if(d_a < candidate_A_deg){
        if(d_a < A_deg){
            if(!A.empty()){
                for(Vertex* ac : A)
                    ac->data.tabu.AB_idx = -1;
                swap(A, A_candidate);
                candidate_A_deg = A_deg;
            }
            clear_A();
            A_deg = d_a;
        }else{
            candidate_A_deg = d_a;
            A_candidate.clear();
            A_candidate.push_back(a);
        }
        if(d_a == A_deg){
            add_A(a);
        }
        return;
    }
    if(d_a == candidate_A_deg)
        A_candidate.push_back(a);
}

void AMTS::update_B(Vertex* b){
    const int d_b = b->data.tabu.dS;
    if(d_b > candidate_B_deg){
        if(d_b > B_deg){
            if(!B.empty()){
                for(Vertex* bc : B)
                    bc->data.tabu.AB_idx = -1;
                swap(B, B_candidate);
                candidate_B_deg = B_deg;
            }
            clear_B();
            B_deg = d_b;
        }else{
            candidate_B_deg = d_b;
            B_candidate.clear();
            B_candidate.push_back(b);
        }
        if(d_b == B_deg){
            add_B(b);
        }
        return;
    }
    if(d_b == candidate_B_deg)
        B_candidate.push_back(b);
}

void AMTS::create_AB(Graph& G){
    auto s_time = G.new_timestamp();
    B_deg = -1;
    candidate_B_deg = -1;
    A_deg = numeric_limits<int>::max();
    candidate_A_deg = numeric_limits<int>::max();

    // compute A and B
    for(Vertex* s : S){
        for(Vertex* n: s->neighbors){
            n->marked = max(n->marked + 1, s_time);
            const int d_n = (int) 1 + (n->marked - s_time);
            degS(n) = d_n;
            if(!TABU(n)){
                if(not_in_S(n)){
                    if (d_n > B_deg){
                        B_deg = d_n;
                        clear_B();
                    }
                    if(d_n == B_deg)
                        add_B(n);
                    //update_B(n);
                }else{
                    if(d_n < A_deg){
                        A_deg = d_n;
                        clear_A();
                    }
                    if(d_n == A_deg)
                        add_A(n);
                    //update_A(n);
                }
            }
        }
    }
    G.increase_timestamp(k+1);
}

void AMTS::clear_A(){
    for(Vertex* a : A)
        a->data.tabu.AB_idx = -1;
    A.clear();
}

void AMTS::add_A(Vertex * a){
    assert(a->data.tabu.s_idx != -1);
    assert(a->data.tabu.AB_idx == -1);
    a->data.tabu.AB_idx = A.size();
    A.push_back(a);
}

void AMTS::add_B(Vertex * b){
    assert(b->data.tabu.s_idx == -1);
    assert(b->data.tabu.AB_idx == -1);
    b->data.tabu.AB_idx = B.size();
    B.push_back(b);
}

void AMTS::pop_A(Vertex* a){
    //if(a->data.tabu.AB_idx == -1)
    //    return;
    A.back()->data.tabu.AB_idx = a->data.tabu.AB_idx;
    swap(A[a->data.tabu.AB_idx], A.back());
    a->data.tabu.AB_idx = -1;
    A.pop_back();
}

void AMTS::pop_B(Vertex* b){
    //if(b->data.tabu.AB_idx == -1)
    //    return;
    B.back()->data.tabu.AB_idx = b->data.tabu.AB_idx;
    swap(B[b->data.tabu.AB_idx], B.back());
    b->data.tabu.AB_idx = -1;
    B.pop_back();
}


void AMTS::clear_B(){
    for(Vertex* b : B)
        b->data.tabu.AB_idx = -1;
    B.clear();
}

void AMTS::clear_Set(vector<Vertex*>& Set){
    for(Vertex* x : Set){
        x->data.tabu.AB_idx = -1;
    }
    Set.clear();
}

void AMTS::pop_Set(vector<Vertex*>& Set, Vertex* x){
    Set.back()->data.tabu.AB_idx = x->data.tabu.AB_idx;
    swap(Set[x->data.tabu.AB_idx], Set.back());
    x->data.tabu.AB_idx = -1;
    Set.pop_back();
}

void AMTS::add_Set(vector<Vertex*>& Set, Vertex* x){
    //assert(x->data.tabu.s_idx == -1);
    assert(x->data.tabu.AB_idx == -1);
    x->data.tabu.AB_idx = Set.size();
    Set.push_back(x);
}

void AMTS::update_AB_new(Graph& G, pair<Vertex*, Vertex*> move){
    auto& [u, v] = move;

    int d_u = 0;
    // u is moved out of S
    /* update u neighborhood */
    for(Vertex* n : u->neighbors){
        d_u += in_S(n);
        n->data.tabu.dS--; // has one less neighbor in S;
        if(n->data.tabu.AB_idx != -1){
            assert(!TABU(n));
            if(in_S(n)){
                if(!A.empty()){
                    swap(A, A_candidate);
                    candidate_A_deg = A_deg;
                }
                clear_Set(A); // new better A, n is only new member
                A_deg = degS(n);
                add_Set(A, n);
            }
            else{
                // this will not work, because this could also be just B_candidate
                if(degS(n)+ 1 == B_deg){
                    pop_Set(B, n); // no longer max
                }else{
                    pop_Set(B_candidate, n);
                    if(B_candidate.empty())
                        candidate_B_deg = -1;
                }
                if(degS(n) == candidate_B_deg)
                    add_Set(B_candidate, n);
                else if(degS(n) > candidate_B_deg){
                    clear_Set(B_candidate);
                    add_Set(B_candidate, n);
                    candidate_B_deg = degS(n);
                }
                
                if(B.empty()){
                    swap(B, B_candidate);
                    B_deg = candidate_B_deg;
                    candidate_B_deg = -1;
                }
            }
        }else if(in_S(n)){
            if(degS(n) < candidate_A_deg){
                clear_Set(A_candidate);
                candidate_A_deg = degS(n);
            }
            if(degS(n) == candidate_A_deg)
                add_Set(A_candidate, n);
        }
    }
    /* update for u */
    u->data.tabu.dS = d_u; // neighbors of u in S

    // v is moved into S
    int d_v = 0;
    /* update v neighborhood */
    for(Vertex* w : v->neighbors){
        d_v += in_S(w);
        w->data.tabu.dS++; // has one less neighbor in S;
        if(w->data.tabu.AB_idx != -1){
            assert(!TABU(w));
            if(in_S(w)){
                if(degS(w) - 1 == A_deg){
                    pop_Set(A, w); // no longer minimum
                }else{
                    pop_Set(A_candidate, w);
                    if(A_candidate.empty())
                        candidate_A_deg = numeric_limits<int>::max();
                }
                if(degS(w) == candidate_A_deg)
                    add_Set(A_candidate, w);
                else if(degS(w) < candidate_A_deg){
                    clear_Set(A_candidate);
                    add_Set(A_candidate, w);
                    candidate_A_deg = degS(w);
                }
                
                if(A.empty()){
                    swap(A, A_candidate);
                    A_deg = candidate_A_deg;
                    candidate_A_deg = numeric_limits<int>::max();
                }
                
            }
            else{
                if(!B.empty()){
                    swap(B, B_candidate);
                    candidate_B_deg = B_deg;
                }
                clear_Set(B); // new best B, w only member
                B_deg = w->data.tabu.dS;
                add_Set(B, w);
            }
        }else if(not_in_S(w)){
            if(degS(w) < candidate_B_deg){
                clear_Set(B_candidate);
                candidate_B_deg = degS(w);
            }
            if(degS(w) == candidate_B_deg)
                add_Set(B_candidate, w);
        }
    }

    v->data.tabu.dS = d_v;
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
            assert(!TABU(n));
            if(in_S(n)){
                clear_Set(A); // new better A, n is only new member
                A_deg = n->data.tabu.dS;
                add_Set(A, n);
            }
            else{
                pop_Set(B, n); // no longer max
            }
        }
    }
    /* update for u */
    u->data.tabu.dS = d_u; // neighbors of u in S

    // v is moved into S
    int d_v = 0;
    /* update v neighborhood */
    for(Vertex* w : v->neighbors){
        d_v += in_S(w);
        w->data.tabu.dS++; // has one less neighbor in S;
        if(w->data.tabu.AB_idx != -1){
            assert(!TABU(w));
            if(in_S(w)){
                pop_Set(A, w); // no longer minimum
            }
            else{
                clear_Set(B); // new best B, w only member
                B_deg = w->data.tabu.dS;
                add_Set(B, w);
            }
        }
    }

    v->data.tabu.dS = d_v;
    
    // either A or B empty, adjust max/min deg accordingly and fill them again
    /*bool fill_A = A.empty();
    bool fill_B = B.empty();

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
                else{ if(not_in_S(v) && fill_B){
                    if(v->data.tabu.dS > new_max_d){
                        new_B.clear();
                        new_max_d = v->data.tabu.dS;
                    }
                    if(new_max_d == v->data.tabu.dS){
                        new_B.push_back(v);
                    }
                }}
            }
        }
        if(fill_A){
            A = new_A;
            min_s_d = new_min_d;
            for(size_t i = 0; i < A.size(); i++){
                assert(in_S(A[i]));
                A[i]->data.tabu.AB_idx = i;
            }
        }
        if(fill_B){
            B = new_B;
            max_d = new_max_d;
            for(size_t i = 0; i < B.size(); i++){
                assert(not_in_S(B[i]));
                B[i]->data.tabu.AB_idx = i;
            }
        }

    }*/
    _profile_fill_AB(G);
    
}

void AMTS::_profile_fill_AB(Graph& G){
     // either A or B empty, adjust max/min deg accordingly and fill them again
    bool fill_A = A.empty();
    bool fill_B = B.empty();

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
                else{ if(not_in_S(v) && fill_B){
                    if(v->data.tabu.dS > new_max_d){
                        new_B.clear();
                        new_max_d = v->data.tabu.dS;
                    }
                    if(new_max_d == v->data.tabu.dS){
                        new_B.push_back(v);
                    }
                }}
            }
        }
        if(fill_A){
            A = new_A;
            A_deg = new_min_d;
            for(size_t i = 0; i < A.size(); i++){
                assert(in_S(A[i]));
                A[i]->data.tabu.AB_idx = i;
            }
        }
        if(fill_B){
            B = new_B;
            B_deg = new_max_d;
            for(size_t i = 0; i < B.size(); i++){
                assert(not_in_S(B[i]));
                B[i]->data.tabu.AB_idx = i;
            }
        }

    }
}

pair<bool, pair<Vertex*, Vertex*>> AMTS::constrained_move(Graph& G){
    
    if(A.empty() || B.empty())
        return {false, {nullptr, nullptr}};

    vector<pair<Vertex*, Vertex*>> T_moves;

    for(Vertex* b : B){
        //assert(b->data.tabu.s_idx != -1);
        auto adj_b = G.new_timestamp();
        for(Vertex* n : b->neighbors){
            n->marked = adj_b;
        }

        for(Vertex* a : A){
            if(a->marked != adj_b){
                T_moves.push_back({a, b});
                assert(b->data.tabu.s_idx == -1);
            }
        }

        if(T_moves.size() > 1000)
            break;
    }

    //cout << "A: " << A.size() << ", B: " << B.size() << ", T: " << T_moves.size() << ", Searchspace: " << A.size() * B.size() << "\n";
 
    bool improving = true;
    
    if(T_moves.empty()){
        improving = ((B_deg - A_deg - 1) > 0);
    }else{
        improving = (B_deg - A_deg) > 0;
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
        assert(u->data.tabu.s_idx != -1);
        assert(v->data.tabu.s_idx == -1);
        return {improving, {u, v}};
    }
}

pair<Vertex*, Vertex*> AMTS::probability_move(Graph& G){
    std::uniform_int_distribution<int> S_distr(0, S.size()-1);
    Vertex* u = S[S_distr(G.gen)];
    const int threshold = max(1, (int) floor(density * k));
    for(Vertex* v : G.V){
        if((v->data.tabu.s_idx == (-1)) && (v->data.tabu.dS <= threshold)){
            //cout << v->data.tabu.s_idx << "\n";
            assert(v->data.tabu.s_idx == -1);
            return {u, v};
        }
    }

    int min_edges = numeric_limits<int>::max();
    Vertex* best = nullptr;
    for(Vertex* v : G.V){
        if(v->data.tabu.s_idx == -1 && v->data.tabu.dS < min_edges && !TABU(v)){
            best = v;
            min_edges = v->data.tabu.dS;
        }
    }

    if(best){
        return {u, best};
    }

    int min_edges_allow_tabu = numeric_limits<int>::max();
    Vertex* best_allow_tabu = nullptr;
    for(Vertex* v : G.V){
        if(v->data.tabu.s_idx == -1 && v->data.tabu.dS < min_edges_allow_tabu){
            best_allow_tabu = v;
            min_edges_allow_tabu = v->data.tabu.dS;
        }
    }

    return {u, best_allow_tabu};

    assert(false);
    return {nullptr, nullptr};
}

