#include "graph.h"

Vertex* Graph::add_vertex(){
    return add_vertex(total_N);
}

Vertex* Graph::add_vertex(size_t id){
    
    Vertex *v = new Vertex;
    v->id = id;
    v->v_idx = V.size();
    V.push_back(v);
    total_N++;
    update_deglists(v);
    return v;
}

void Graph::delete_vertex(Vertex* v){
    for(Edge* e : v->edges){
        #if DEBUG
            //cout << "deleting " << v->name << " (" << e->ends[0].v->name << ", " << e->ends[1].v->name << ")\n";
        #endif
        delete_edge_lazy(e, v);
    }
    N--;
    // also remove v from V?
    delete_from_deglist(v);
}

Vertex* Graph::MM_add_vertex(){
    Vertex* v = new Vertex;
    v->id = -1;
    v->list_idx = -1;
    N++;
    v->v_idx = V.size();
    V.push_back(v);
    return v;
}

void Graph::MM_discard_vertex(Vertex* v){
    delete_vertex(v);
    assert(v->status == UNKNOWN);
    v->status = EXCLUDED;
    auto op = new OP_DiscardVertex(v);
    history.emplace_back(op);
}

void Graph::MM_clisat_add_vertex(Vertex* v){
    v->status = CLIQUE;
    partial.push_back(v);
    history.emplace_back(new OP_AddCli(v));
}

void Graph::MM_updated_mu(vector<pair<Vertex*, int>>& old_mu_values){
    history.emplace_back(new OP_UpdatedMu(old_mu_values));
}




void Graph::MM_vc_add_vertex(Vertex* v){
    delete_vertex(v);
    assert(v->status == UNKNOWN);
    v->status = VC;
    k--;
    sol_size++;
    auto op = new OP_SelectVertex(v);
    history.emplace_back(op);
}

void Graph::MM_vc_add_neighbors(Vertex* v){
    size_t d = deg(v);
    k -= d;
    sol_size += d;

    vector<Vertex*> selected(d);

    size_t i = 0;
    while(!v->neighbors.empty()){
        Vertex* n = v->neighbors.back();
        //if(USE_PACK)
        //    add_selected_constraint(*this, n);
        n->status = VC;
        delete_vertex(n);
        selected[d - 1 - i] = n;
        i++;
    }
    assert(v->status == UNKNOWN);
    //v->status = EXCLUDED;

    history.emplace_back(new OP_SelectVertex(selected));
    MM_discard_vertex(v);

    //discard_vertex(v);
}


void Graph::MM_clique_add_vertex(Vertex* v)
{
    assert(v->status == UNKNOWN);
    v->status = CLIQUE;
    partial.push_back(v);
    k--;
    sol_size++;
    history.push_back(new OP_AddClique());
}

bool Graph::MM_clique_add_vertex_if_valid(Vertex* candidate){
    assert(candidate->status == UNKNOWN);
    new_timestamp();
    for(Vertex* c : partial){
        c->marked = timestamp;
    }

    size_t count = 0;
    for(Vertex* n : candidate->neighbors){
        count += (n->marked == timestamp);

        if(count == partial.size()){
            candidate->status = CLIQUE;
            partial.push_back(candidate);
            k--;
            sol_size++;
            history.push_back(new OP_AddClique());
            return true;
        }
    }
    return false;
}

void Graph::restore_vertex(Vertex* v){
    v->status = UNKNOWN;
    N++;

    assert(v->neighbors.size() == v->edges.size());

    for(size_t i = 0; i < v->neighbors.size(); i++){
        Vertex* u = v->neighbors[i];
        Edge* e = v->edges[i];

        size_t side = (e->ends[0].v == u)? 0 : 1;
        e->ends[side].idx = u->edges.size();
        u->edges.push_back(e);
        u->neighbors.push_back(v);

        update_deglists(u);

        M++;

        #if AUTO_DEG0
        // if u was deg 0, it is now restored
        if(deg(u) == 1){
            N++;
            u->status = UNKNOWN;
        }
        #endif
    }
    update_deglists(v);
}

void Graph::MM_induced_subgraph(vector<Vertex*>& induced_set){
    history.push_back(new OP_InducedSubgraph(induced_set, *this));
}

void Graph::MM_induced_neighborhood(Vertex* v){
    vector<Vertex*> nv = v->neighbors;
    nv.push_back(v);
    MM_induced_subgraph(nv);
}

Edge* Graph::add_edge(Vertex* u, Vertex* v){
    assert(!u->adjacent_to(v));
    Edge *e = new Edge;
    e->ends[0] = {u, static_cast<size_t>(deg(u))};
    e->ends[1] = {v, static_cast<size_t>(deg(v))};

    e->idx = E.size();
    E.push_back(e);
    M++;

    u->neighbors.push_back(v);
    v->neighbors.push_back(u);

    u->edges.push_back(e);
    v->edges.push_back(e);

    update_deglists(u);
    update_deglists(v);
    return e;
}

Edge* Graph::MM_add_edge(Vertex* u, Vertex* v){
    Edge* e = add_edge(u,v);
    history.push_back(new OP_AddEdge(e));
    return e;
}

void Graph::pop_edge(size_t edge_idx){
    E.back()->idx = edge_idx;
    swap(E[edge_idx], E.back());
    E.pop_back();
}

void Graph::pop_edge(Edge* e){
    E.back()->idx = e->idx;
    swap(E[e->idx], E.back());
    E.pop_back();
}

void Graph::pop_vertex(Vertex* v){
    V.back()->v_idx = v->v_idx;
    swap(V[v->v_idx], V.back());
    V.pop_back();
}

void Graph::delete_edge(Edge* e){
    delete remove_edge(e);
}

[[nodiscard]] Edge* Graph::remove_edge(Edge* e){
    for(auto& end : e->ends){
        Vertex* u = end.v;

        swap(u->neighbors[end.idx], u->neighbors.back());
        u->neighbors.pop_back();

        Edge* back_edge = u->edges.back();
        size_t side = (u == back_edge->ends[0].v) ? 0 : 1; // updates the edge with the new position
        back_edge->ends[side].idx = end.idx;

        swap(u->edges[end.idx], u->edges.back());
        u->edges.pop_back();

        M--;

        update_deglists(u);

        #if AUTO_DEG0
        if(deg(u) == 0 && u->status == UNKNOWN){
            u->status = EXCLUDED; // add some explicit deg0_removal op for clarity?
            N--;
        }
        #endif
    }

    E.back()->idx = e->idx;
    swap(E[e->idx], E.back());
    E.pop_back();
    
    return e;
}

void Graph::delete_edge_lazy(Edge* e, Vertex* v){
    
    //get endpoint that is not on v's side
    Endpoint& end = (e->ends[0].v != v)? e->ends[0] : e->ends[1];

    Vertex* u = end.v;
 
    assert(deg(u) > 0);

    swap(u->neighbors[end.idx], u->neighbors.back());
    u->neighbors.pop_back();
        
    Edge* back_edge = u->edges.back();
    size_t side = (u == back_edge->ends[0].v) ? 0 : 1; // updates the edge with the new position
    back_edge->ends[side].idx = end.idx;

    swap(u->edges[end.idx], u->edges.back());
    u->edges.pop_back();

    update_deglists(u);

    M--;

    #if AUTO_DEG0
    if(deg(u) == 0 && u->status == UNKNOWN){
        u->status = EXCLUDED; // add some explicit deg0_removal op for clarity?
        N--;
    }
    #endif

    if(UPDATE_G_EDGELIST){
        E.back()->idx = e->idx;
        swap(E[e->idx], E.back());
        E.pop_back();
    }
}
// call AFTER adding/removing edges, neighbors etc.
void Graph::update_deglists(Vertex* v){
    [[likely]]if(v->deg_idx != (size_t) -1){
        remove_from_deglist(v);
    }

    max_degree = max(deg(v), max_degree);

    #if USE_MIN_DEG
    min_degree = min(deg(v), min_degree);
    #endif

    [[unlikely]]if(deg(v) >= deg_lists.size()){
        deg_lists.resize(deg(v) + 1); //arbitrary constant >= 1
    }

    v->deg_idx = deg_lists[deg(v)].size();
    deg_lists[deg(v)].push_back(v);
    v->list_idx = deg(v);
}

void Graph::remove_from_deglist(Vertex* v){
    auto& deg = deg_lists[v->list_idx];
    deg.back()->deg_idx = v->deg_idx;
    swap(deg[v->deg_idx], deg.back());
    deg.pop_back();

    //adjust max_degree
    if(v->list_idx == max_degree && deg.empty()){
        assert(deg_lists.size() > 0);
        for(int i = static_cast<int>(deg_lists.size()) - 1; i >= 0; i--){
            if(!deg_lists[i].empty()){
                max_degree = i;
                if(v->list_idx == min_degree && deg.empty())
                    goto min;
                return;
            }
        }
        max_degree = 0;
        //deg_lists.resize(max_degree);
    }
    #if USE_MIN_DEG
    if(v->list_idx == min_degree && deg.empty()){
        min:
        for(int i = 0; i < static_cast<int>(deg_lists.size()); i++){
            if(!deg_lists[i].empty()){
                min_degree = i;
                return;
            }
        }
        min_degree = numeric_limits<int>::max();
    }
    #endif
}


void Graph::delete_from_deglist(Vertex* v){
    remove_from_deglist(v);
    v->list_idx = -1;
    v->deg_idx = -1;
    #if DEBUG
        cout << "deleted " << v->name << " from lists" << "\n";
    #endif
}


Graph Graph::shallow_copy(){
    Graph H;
    for(Vertex* v : V){
        H.add_vertex(v->id);
    }

    for(Edge* e : E){
        H.add_edge(e->ends[0].v, e->ends[1].v);
    }

    H.name_table = name_table;
    H.N = H.total_N;
    
    return H;
}

Graph Graph::complementary_graph(Graph& G){
    Graph H;
    H.name_table = G.name_table;

    //vector<Vertex*> G_to_H(G.N); // from v in G to v in H
    unordered_map<Vertex*, Vertex*> G_to_H;
    for(Vertex* v : G.V){
        Vertex* vh = H.add_vertex();
        vh->id = v->id;
        //G_to_H[v->id] = vh;
        G_to_H[v] = vh;
    }

    for(Vertex* v : G.V){
        G.new_timestamp();
        auto v_iter = G.timestamp;
        v->marked = v_iter;         // prevent self loops
        for(Vertex* n : v->neighbors){
            n->marked = G.timestamp;
        }

        for(Vertex* w : G.V){
            if(w->marked != v_iter && w < v){
                H.add_edge(G_to_H[v], G_to_H[w]);
            }
        }
    }

    H.N = H.total_N;
    
    //G.delete_all();
    return H;
}

void Graph::delete_all(){
    for(Edge* e : E){
        delete e;
    }
    // for(Vertex* v : V){
    //     while(!v->edges.empty()){
    //         delete_edge(v->edges.back());
    //     }
    // }
    for(Vertex* v : V)
        delete v;
}

void Graph::undo(){
    history.back()->undo(this);
    delete history.back();
    history.pop_back();
}

void Graph::restore(){
    while(typeid(*history.back()) != typeid(OP_RestorePoint)){
        undo();
    }
    undo(); // remove the restore point itself
}

void Graph::set_restore(){
    history.emplace_back(new OP_RestorePoint());
    if(USE_PACK)
        history.emplace_back(new OP_SetPackingPoint(constraints.size()));
}

unsigned long long Graph::new_timestamp(){
    timestamp++;
    return timestamp;
}

void Graph::increase_timestamp(unsigned long long offset){
    timestamp += offset;
}
void Graph::print_op_line(Vertex* v, string name, state before, state after){
    //stringstream ll;
    cout << "# " << name << " = " << v->id << " [ " << enum_name(before) <<", " << enum_name(after) << "]\n";
    //return ll.str();
}

// missing deg2-resolve, so currently not correct (but sol_size is correct)
void Graph::output_vc(){
    for(Vertex* v : partial){
        cout << name_table[v->id] << "\n";
    }
}

void Graph::resolve_vc(bool partial){
    for(Vertex* v : V){
        v->data.resolved_status = v->status;
    }
    int tcount = 0;
    #if DEBUG_OPS
        cout << "\n\n\n #### NEW RESOLVE VC\n\n\n";
    #endif
    for(auto iter = transform_access.rbegin(); iter != transform_access.rend(); iter++){
        #if DEBUG_OPS
        (*iter)->debug_print(this, partial);
        #else
        (*iter)->resolve(this, partial);
        #endif
        tcount++;
    }
}

void Graph::set_current_vc(){

    for(Vertex* v : V){
        v->data.resolved_status = v->status;
    }
    
    for(auto iter = transform_access.rbegin(); iter != transform_access.rend(); iter++){
        (*iter)->resolve(this, false);
    }
   

    partial.clear();
    for(Vertex* v : V){
        //assert(v->data.resolved_status != UNKNOWN);
        if(v->data.resolved_status == VC)
            partial.push_back(v);
    }
}


bool Vertex::adjacent_to(Vertex* u){
    Vertex* v = this; 
    // v has smaller neighborhood
    if(deg(v) > deg(u))
        swap(v, u);

    for(Vertex* n: v->neighbors){
        if(n == u)
            return true;
    }
    return false;
}