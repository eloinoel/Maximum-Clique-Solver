#include "graph.h"

Vertex* Graph::add_vertex(){
    return add_vertex(N);
}

Vertex* Graph::add_vertex(size_t id){
    Vertex *v = new Vertex;
    v->id = id;
    V.push_back(v);
    N++;
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

void Graph::MM_discard_vertex(Vertex* v){
    delete_vertex(v);
    assert(v->status == UNKNOWN);
    v->status = EXCLUDED;
    auto op = new OP_DiscardVertex(v);
    history.emplace_back(op);
}

void Graph::MM_select_vertex(Vertex* v){
    delete_vertex(v);
    assert(v->status == UNKNOWN);
    v->status = INCLUDED;
    k--;
    sol_size++;
    auto op = new OP_SelectVertex(v);
    history.emplace_back(op);
}

void Graph::MM_select_neighbors(Vertex* v){
    size_t deg = v->degree();
    k -= deg;
    sol_size += deg;

    vector<Vertex*> selected(deg);

    size_t i = 0;
    while(!v->neighbors.empty()){
        Vertex* n = v->neighbors.back();
        n->status = INCLUDED;
        delete_vertex(n);
        selected[deg - 1 - i] = n;
        i++;
    }
    

    history.emplace_back(new OP_SelectVertex(selected));

    //discard_vertex(v);
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

        // if u was deg 0, it is now restored
        if(u->degree() == 1){
            N++;
            u->status = UNKNOWN;
        }
    }
    update_deglists(v);
}

Edge* Graph::add_edge(Vertex* u, Vertex* v){
    Edge *e = new Edge;
    e->ends[0] = {u, static_cast<size_t>(u->degree())};
    e->ends[1] = {v, static_cast<size_t>(v->degree())};

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

void Graph::delete_edge(Edge* e){
    for(auto& end : e->ends){
        Vertex* u = end.v;

        swap(u->neighbors[end.idx], u->neighbors.back());
        u->neighbors.pop_back();

        Edge* back_edge = u->edges.back();
        size_t side = (u == back_edge->ends[0].v) ? 0 : 1; // updates the edge with the new position
        back_edge->ends[side].idx = end.idx;

        swap(u->edges[end.idx], u->edges.back());
        u->edges.pop_back();

        update_deglists(u);

        if(u->degree() == 0 && u->status == UNKNOWN){
            u->status = EXCLUDED; // add some explicit deg0_removal op for clarity?
            N--;
        }
    }

    E.back()->idx = e->idx;
    swap(E[e->idx], E.back());
    E.pop_back();

    delete e;
    
}

void Graph::delete_edge_lazy(Edge* e, Vertex* v){
    for(auto& end : e->ends){
        Vertex* u = end.v;

        if(u == v)
            continue;   

        #if DEBUG
            cout << u->name << "\n";
        #endif
        assert(u->degree() > 0);

        swap(u->neighbors[end.idx], u->neighbors.back());
        u->neighbors.pop_back();
        

        Edge* back_edge = u->edges.back();
        size_t side = (u == back_edge->ends[0].v) ? 0 : 1; // updates the edge with the new position
        back_edge->ends[side].idx = end.idx;

        swap(u->edges[end.idx], u->edges.back());
        u->edges.pop_back();


       
        update_deglists(u);
        if(u->degree() == 0 && u->status == UNKNOWN){
            u->status = EXCLUDED; // add some explicit deg0_removal op for clarity?
            N--;
        }
    }

    if(UPDATE_G_EDGELIST){
        E.back()->idx = e->idx;
        swap(E[e->idx], E.back());
        E.pop_back();
    }
}
// call AFTER adding/removing edges, neighbors etc.
void Graph::update_deglists(Vertex* v){
    if(v->deg_idx != (size_t) -1){
        remove_from_deglist(v);
    }

    max_degree = max(v->degree(), max_degree);

    if(v->degree() >= deg_lists.size()){
        deg_lists.resize(v->degree() + 5); //arbitrary constant >= 1
    }

    v->deg_idx = deg_lists[v->degree()].size();
    deg_lists[v->degree()].push_back(v);
    v->list_idx = v->degree();
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
                return;
            }
        }
        max_degree = 0;
    }
}


void Graph::delete_from_deglist(Vertex* v){
    remove_from_deglist(v);
    v->list_idx = -1;
    v->deg_idx = -1;
    #if DEBUG
        cout << "deleted " << v->name << " from lists" << "\n";
    #endif
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
}

void Graph::new_timestamp(){
    timestamp++;
}

void Graph::increase_timestamp(unsigned long long offset){
    timestamp += offset;
}


// missing deg2-resolve, so currently not correct (but sol_size is correct)
void Graph::output_vc(){
    for(Vertex* v : V){
        if(v->status == INCLUDED)
            cout << name_table[v->id] << "\n";
    }
}

size_t Vertex::degree() const {
    return neighbors.size();
}

bool Vertex::adjacent_to(Vertex* u){
    Vertex* v = this; 
    // v has smaller neighborhood
    if(v->degree() > u->degree())
        swap(v, u);

    for(Vertex* n: v->neighbors){
        if(n == u)
            return true;
    }
    return false;
}