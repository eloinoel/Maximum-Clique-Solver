#include "few_common_neighbors.h"
#include "./../orderings/degeneracy_ordering.h"
#include "lower_bounds.h"
#include "k_core.h"
#include "AMTS.h"

void zero_dependencies(Graph& G){
    for(Vertex* v : G.V){
        v->data.has_dependency = false;
    }
}

vector<string> reduce(Graph& G,  unordered_map<string, state>& sol, vector<_recover_unit>& rec){
    vector<string> init_clique = get_LB_wrapper(G);
    G.LB = init_clique.size();
    //cout << "LB = " << LB << "\n";

    int old_N = G.N;

    AMTS amts = AMTS(G);

    vector<string> amts_clique = amts.find_best(G.LB, 500, G);
    if(amts_clique.size() > init_clique.size())
        init_clique = amts_clique;

    //cout << "lower bound " << G.LB << "\n";
    //cout << "amts clique " << amts_clique.size() << "\n";
    //cout << "init clique " << init_clique.size() << "\n";

    if(init_clique.size() > G.LB)
        G.LB = init_clique.size();

    do{
        old_N = G.N;
        apply_k_core(G, G.LB);
        few_common_rule(G, G.LB);
        apply_k_core(G, G.LB);
        universal_rule(G, sol);
        comp_deg1_rule(G, sol);
        //comp_deg2_rule(G, sol, rec); broken for some reason
        apply_k_core(G, G.LB);
        few_common_rule(G, G.LB);
        apply_k_core(G, G.LB);
        //cout << "[d] " << G.N << "\n";
        if(G.N < 10000)
            domination_comp_rule(G);
        //cout << "[r] " << G.N << "\n";

    }while(old_N != G.N);

    return init_clique;

}

vector<string> get_LB_wrapper(Graph& G){
    auto result = degeneracy_ordering_rN(G);
    //check lower and upper bounds
    auto LB_maximum_clique = degeneracy_ordering_LB(move(result.first), move(result.second));
    vector<string> lb_s;
    for(auto& v : LB_maximum_clique){
        lb_s.push_back(G.name_table[v->id]);
    }
    return lb_s;
}


void few_common_rule(Graph& G, int LB){
    if(LB < 0)
        LB = G.LB;

    int _deleted_edges = 0;

    for(int i = G.min_degree; i <= G.max_degree; i++){
        for(Vertex* v : G.deg_lists[i]){
            auto adj_v = G.new_timestamp();
            for(Vertex* n : v->neighbors){
                n->marked = adj_v;
            }

            int j = 0;
            while(j < deg(v)){
                Edge* e = v->edges[j];
                Vertex* n = e->other_end_of(v);
                int needed = LB-1;
                int max = deg(n);
                for(Vertex* w : n->neighbors){
                    needed -= (w->marked == adj_v);
                    max--;
                    if(needed > max || needed == 0)
                        break;
                }

                if(needed > 0){
                    //delete edge
                    G.delete_edge(e);
                    _deleted_edges++;
                    continue; // don't increment as deg(v) is reduced by 1 already
                }

                j++;
            }
        }
    }
    //cout << "deleted total of " << _deleted_edges << "\n";
}

void sparse_neighbor_rule(Graph& G, int LB){
    if(LB < 0)
        LB = G.LB;

    const int required = (LB * (LB+1)); // edges are counted twice, 2 * edges requred for LB+1-size clique
    for(int i = G.min_degree; i <= G.max_degree; i++){
        int j = 0;
        while(j < G.deg_lists[i].size()){
            Vertex* v = G.deg_lists[i][j];
            auto adj_v = G.new_timestamp();
            int total_remaining = 0;

            for(Vertex* n : v->neighbors){
                n->marked = adj_v;
                total_remaining += deg(n);
            }
            v->marked = adj_v;

            int edges_among_N = deg(v);  // v is adjacent to all its neighbors, skips iterating them again
            int k = 0;
            while(k < deg(v)){
                Edge* e = v->edges[k];
                Vertex* n = e->other_end_of(v);
                int induced_degree = 0;
                for(Vertex* w : n->neighbors){
                    edges_among_N += (w->marked == adj_v);
                    induced_degree += (w->marked == adj_v);
                }
                total_remaining -= deg(n);
                if(induced_degree < LB){
                    //cout << "induced degree " << induced_degree << "\n";
                    G.delete_edge(e);
                }else{
                    k++;
                }
                //if (total_remaining + edges_among_N < required){
                //    break;
                //}
            }
            //cout << "deg: "<< deg(v) << ", r: " << required << ", got: " << edges_among_N << "\n";
            if(edges_among_N < required || deg(v) < LB){
                G.delete_vertex(v);
            }
            else{
                LB++;
                j++;
            }
        }
    }
}



void domination_comp_rule(Graph& G){
    for(int i = G.min_degree; i <= G.max_degree; i++){
        int j = 0;
        while(j < G.deg_lists[i].size()){
            Vertex* v = G.deg_lists[i][j];
            auto adj_v = G.new_timestamp();
            v->marked = adj_v;
            for(Vertex* n : v->neighbors){
                n->marked = adj_v;
            }
            
            for(int i_ = G.min_degree; i_ < deg(v); i_++){
                int j_ = 0;
                while(j_ < G.deg_lists[i_].size()){
                    Vertex* w = G.deg_lists[i_][j_];
                    if(deg(w) >= deg(v)){
                        //cout << "inconsistent degrees\n";
                    }
                    if(w->marked == adj_v){
                        j_++;
                        continue;
                    }
                    bool dominated = true;
                    for(Vertex* nw : w->neighbors){
                        if(nw->marked != adj_v){
                            dominated = false;
                            break;
                        }
                    }

                    if(dominated){
                        G.delete_vertex(w);
                    }else{
                        j_++;
                    }
                }
            }
            j++;
        }
    }
}

class CLIQUE_add_by_reduction : public Operation{
    Vertex* added;
public:
    CLIQUE_add_by_reduction(Vertex* _added, Graph& G)
     : added(_added) {
        added->data.has_dependency = true;
        G.delete_vertex(added);
        added->status = CLIQUE;
        G.LB--;
        G.sol_size++;
     }

     void undo (Graph* G) const{
        G->restore_vertex(added);
        //added->status = UNKNOWN;
        G->LB++;
        G->sol_size--;
     }
};

void universal_rule(Graph& G, unordered_map<string, state>& sol){
    while(G.max_degree == G.N - 1){
        Vertex* v = G.deg_lists[G.max_degree].front();
        //cout << "added " << v->id << " to clique\n";
        G.history.push_back(new CLIQUE_add_by_reduction(v, G));
        sol[G.name_table[v->id]] = CLIQUE;
    }
}

void comp_deg1_rule(Graph& G, unordered_map<string, state>& sol){
    while(G.max_degree >= G.N - 2 &&  !G.deg_lists[G.N - 2].empty()){
        Vertex* v = G.deg_lists[G.N-2].front();
        auto adj_v = G.new_timestamp();

        v->marked = adj_v;
        for(Vertex* n : v->neighbors)
            n->marked = adj_v;
        for(int i = G.min_degree; i <= G.max_degree; i++){
           for(Vertex* x : G.deg_lists[i]){
                if(x->marked != adj_v){
                    G.delete_vertex(x);
                    goto found;
                }
           }
        }
        found:
        G.history.push_back(new CLIQUE_add_by_reduction(v, G));
        sol[G.name_table[v->id]] = CLIQUE;
        //cout << "added " << v->id << " to clique\n";
    }
}

class CLIQUE_deg2_fold : public VC_Transformation{
public:
    Vertex* folded;     // folded, the deg2 vertex - 'v'
    Vertex* discard;    // discarded, the vertex that will not stay after merge 
    Vertex* merged;     // merged, the vertex that gets merged with discard and stays in the grap

    vector<Edge*> deleted_from_merged;
    
    // this pattern by aleksander of executing the rule while creating the operation is pretty neat I think
    CLIQUE_deg2_fold(Vertex* _folded, Vertex* u1, Vertex* u2, Graph& G)
     : folded(_folded), discard(u1), merged(u2) {
        G.transform_access.push_back(this);

        /* need to keep these around */
        folded->data.has_dependency = true;
        discard->data.has_dependency = true;
        merged->data.has_dependency = true;

        G.delete_vertex(folded); //note: state stays UNKNOWN

        //discard has larger neighborhood
        if(deg(discard) < deg(merged))
            swap(discard,merged);

        G.new_timestamp();

        cout << "fold is " << G.name_table[folded->id] << "\n";
        cout << "discard is " << G.name_table[discard->id] << "\n";
        cout << "merged is " << G.name_table[merged->id] << "\n";

        for(Vertex* n : discard->neighbors)
            n->marked = G.timestamp;

    
        int j = 0;
        while(j < deg(merged)){
            Edge* e = merged->edges[j];
            Vertex* n = e->other_end_of(merged);

            if(n->marked != G.timestamp){
                deleted_from_merged.push_back(G.remove_edge(e));
            }else{
                j++;
            }
        }

        G.delete_vertex(discard);

        G.LB--;
        G.sol_size++;
    }

    void undo (Graph* G) const{
        G->transform_access.pop_back();
        G->sol_size--;
        G->LB++;
        G->restore_vertex(discard);
        for(auto it = deleted_from_merged.rbegin(); it != deleted_from_merged.rend(); it++){
            Edge* e = *it;
            merged->push_edge(e);
            e->other_end_of(merged)->push_edge(e);
        }
        G->restore_vertex(folded);
    }

    void resolve(Graph* G, bool partial) {
        assert(folded->data.resolved_status == UNKNOWN);
        assert(discard->data.resolved_status == UNKNOWN);
        
        if(merged->data.resolved_status == CLIQUE){
            discard->data.resolved_status = CLIQUE;
            folded->data.resolved_status = EXCLUDED;
            return;
        }
        if(merged->data.resolved_status == EXCLUDED){
            discard->data.resolved_status = EXCLUDED;
            folded->data.resolved_status = CLIQUE;
            return;
        }
        if(partial)
            return;
        assert(false && "error clique deg2");
    }

    void debug_print(Graph* G, bool partial){
        cout << "#### CLIQUE DEG 2 FOLD\n";
        state fs = folded->data.resolved_status;
        state ds = discard->data.resolved_status;
        state ms = merged->data.resolved_status;
        resolve(G, partial);
        state after_fs = folded->data.resolved_status;
        state after_ds = discard->data.resolved_status;
        state after_ms = merged->data.resolved_status;

        G->print_op_line(folded , "folded" , fs, after_fs);
        G->print_op_line(discard, "discard", ds, after_ds);
        G->print_op_line(merged , "merged" , ms, after_ms);
        cout <<"#### DONE\n";
    }
};

void comp_deg2_rule(Graph& G, unordered_map<string, state>& sol, vector<_recover_unit>& rec_sol){
    while(G.max_degree >= G.N - 3 && !G.deg_lists[G.N - 3].empty()){
        Vertex* v = G.deg_lists[G.N-3].front();
        auto adj_v = G.new_timestamp();
        v->marked = adj_v;
        for(Vertex* n : v->neighbors)
            n->marked = adj_v;
        
        int found_missing = 0;
        Vertex* found[2];

        for(int i = G.min_degree; i <= G.max_degree; i++){
           for(Vertex* x : G.deg_lists[i]){
                if(x->marked != adj_v){
                    found[found_missing++] = x;
                    if(found_missing == 2)
                        goto found_all;
                }
           }
        }

        assert(false && "error in deg-2 rule"); // if somehow did not find 2 missing neighbors

        found_all:
        //cout << "deg2 added something\n";
        if(found[0]->adjacent_to(found[1])){
            //cout << "case triangle " << "\n";
            G.MM_discard_vertex(found[0]);
            G.MM_discard_vertex(found[1]);

            G.history.push_back(new CLIQUE_add_by_reduction(v, G));
            sol[G.name_table[v->id]] = CLIQUE;
        }else{
            G.history.push_back(new CLIQUE_deg2_fold(v, found[0], found[1], G));
            string f = G.name_table[v->id];
            string m = ( deg(found[1]) < deg(found[0]) )? G.name_table[found[1]->id] : G.name_table[found[0]->id];
            string d = ( deg(found[1]) > deg(found[0]) )? G.name_table[found[1]->id] : G.name_table[found[0]->id];
            rec_sol.push_back({f, m ,d});
            cout << "########################\n";
            cout << "f is " << f << "\n";
            cout << "d is " << d << "\n";
            cout << "m is " << m << "\n";
            }

    }
}
