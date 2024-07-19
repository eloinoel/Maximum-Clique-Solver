#include "k_truss.h"
#include "graph.h"
#include "colors.h"

using namespace std;

KTruss::KTruss(const Graph& G, std::vector<Vertex*>& degeneracy_ordering)
{
    H = make_shared<Graph>(G.shallow_copy());
    init_edge_map(*(H.get()));

    //block scope to free variable memory when not needed anymore
    {
        std::vector<Triangle> triangles = compute_triangles(degeneracy_ordering);
        //compute support for each edge, i.e. how many triangles it is a part of
        compute_support(triangles, *(H.get()));
    }
    {
        //sort all edges in ascending order of their support
        BucketSort b = BucketSort(edge_support);
        sorted_edges = b.get_sorted_edges();
    }

    // assert correctly sorted
    #if !NDEBUG
    for(int i = 0; i < (int) sorted_edges.size() - 1; ++i)
    {
        assert(support(sorted_edges[i]) <= support(sorted_edges[i + 1]));
    }
    #endif
}

KTruss::~KTruss()
{
    for(edge* e : edge_support)
    {
        assert(e != nullptr);
        delete e;
    }
}

//MARK: K-Classes

void KTruss::compute_k_classes()
{
    k_classes = std::vector<std::vector<int>>();
    int k = 2;
    for(size_t i = 0; i < sorted_edges.size(); ++i)
    {
        edge* e = sorted_edges[i]; //edge with lowest support
        assert(e != nullptr);
        assert(e->support >= 0);
        while(e->support < k - 2)
        {
            k++;
        }

        //we want u to have the smaller degree for efficiency
        assert(e->id >= 0);
        Vertex* u = H->E[e->id]->ends[0].v;
        Vertex* v = H->E[e->id]->ends[1].v;
        if (deg(u) > deg(v))
        {
            std::swap(u, v);
        }

        for(int j = 0; j < (int) u->neighbors.size(); ++j)
        {
            Vertex* w = u->neighbors[j];
            // found triangle --> update supports and remove edge
            if(is_edge(v->id, w->id))
            {
                edge* u_w = get_edge(u->id, w->id);
                edge* v_w = get_edge(v->id, w->id);

                //update support
                decrement_support(u_w);
                decrement_support(v_w);

                //reorder (u,w) and (v, w) according to new support
                reorder_edge(u_w, i);
                reorder_edge(v_w, i);
            }
        }
        //add edge to k-class
        // if((int) k_classes.size() <= k)
        // {
        //     k_classes.resize(k+1);
        // }
        k_classes.push_back(std::vector<int>());
        k_classes[k].push_back(e->id);

        //remove edge from graph
        #if !NDEBUG
        // verify that not already deleted
        removed_edges.find(e->id);
        assert(removed_edges.find(e->id) == removed_edges.end());
        removed_edges[e->id] = true;
        #endif
        (void)H->remove_edge(H->E[e->id]);
    }
    computed_k_classes = true;
}

size_t KTruss::reduce(Graph& G, int lower_clique_bound)
{
    // verify that contents in G and H are the same
    #if !NDEBUG
    for(int i = 0; i < (int) G.E.size(); ++i)
    {
        assert(G.E[i]->ends[0].v->id == H->E[i]->ends[0].v->id);
        assert(G.E[i]->ends[1].v->id == H->E[i]->ends[1].v->id);
    }
    #endif

    size_t num_deleted_edges = 0;
    // deduce from k_classes
    if(computed_k_classes)
    {
        for(int i = 0; i < (int) k_classes.size() && lower_clique_bound - 2; ++i)
        {
            for(int edge_id : k_classes[i])
            {
                #if !NDEBUG
                // verify that not already deleted
                removed_edges.find(edge_id);
                assert(removed_edges.find(edge_id) == removed_edges.end());
                removed_edges[edge_id] = true;
                #endif
                (void)H->remove_edge(G.E[edge_id]);
                num_deleted_edges++;
            }
        }
    }
    // deduce from edge_support
    else
    {
        for(edge* e : sorted_edges)
        {
            if(support(e) < lower_clique_bound - 2)
            {
                #if !NDEBUG
                // verify that not already deleted
                removed_edges.find(e->id);
                assert(removed_edges.find(e->id) == removed_edges.end());
                removed_edges[e->id] = true;
                #endif
                (void)H->remove_edge(G.E[e->id]);
                num_deleted_edges++;
            }
            else // since sorted edges are in ascending order of support, we can break
            {
                break;
            }
        }
    }
    return num_deleted_edges;
}

int KTruss::upper_bound()
{
    if(computed_k_classes)
    {
        return (int) k_classes.size() + 2;
    }
    else
    {
        compute_k_classes();
        return (int) k_classes.size() + 2;
    }
}

void KTruss::print_support(PrintVertices print_vertices)
{
    for(edge* e : edge_support)
    {
        assert(e != nullptr);
        switch(print_vertices)
        {
            case PrintVertices::Names:
                cout << "edge " << GREEN << H->name_table[H->E[e->id]->ends[0].v->id] << " - " << H->name_table[H->E[e->id]->ends[1].v->id] << RESET << " support: " << CYAN << e->support << RESET << endl;
                break;
            case PrintVertices::IDs:
                cout << "edge " << GREEN << H->E[e->id]->ends[0].v->id << " - " << H->E[e->id]->ends[1].v->id << RESET << " support: " << CYAN << e->support << RESET << endl;
                break;
            default:
                break;
        }
    }
}



//MARK: Triangles

inline vector<Vertex*> intersection(vector<Vertex*>& ordered_array1, vector<Vertex*>& ordered_array2, unordered_map<Vertex*, unsigned int>& ordering){
    vector<Vertex*> result;
    for(int i =0 ,j =0; i<(int)ordered_array1.size() && j<(int)ordered_array2.size();){
        if(ordering[ordered_array1[i]]<ordering[ordered_array2[j]])i++;
        else if(ordering[ordered_array1[i]]>ordering[ordered_array2[j]])j++;
        else {
            result.push_back(ordered_array1[i]);
            i++;
            j++;
        }
    }
    return result;
}

std::vector<Triangle> KTruss::compute_triangles(std::vector<Vertex*>& degeneracy_ordering)
{
    vector<Triangle> triangles;
    unordered_map<Vertex*, vector<Vertex*>> A;
    A.reserve(degeneracy_ordering.size());
    unordered_map<Vertex*, unsigned int> ordering;
    for( unsigned int i =0; i<degeneracy_ordering.size(); i++){
        ordering[degeneracy_ordering[i]]=i;
    }
    for(Vertex* v_i:degeneracy_ordering){
        for(Vertex* v_l: v_i->neighbors){
            if(ordering[v_i]<ordering[v_l]){
                for(Vertex* v: intersection(A[v_i], A[v_l], ordering)){
                    Triangle t = {(unsigned int) v->id, (unsigned int) v_i->id,  (unsigned int) v_l->id};
                    triangles.push_back(t);
                }
                A[v_l].push_back(v_i);
            }
        }
    }
    return triangles;
}

//MARK: Edges & Support

void KTruss::init_edge_map(Graph& G)
{
    for(size_t i = 0; i < G.E.size(); ++i)
    {
        unsigned int v0 = G.E[i]->ends[0].v->id;
        unsigned int v1 = G.E[i]->ends[1].v->id;
        edge_map[std::make_pair(v0, v1)] = i;

        #if !NDEBUG
        auto it = edge_map.find(std::make_pair(v0, v1));
        assert(it != edge_map.end());
        it = edge_map.find(std::make_pair(v1, v0));
        assert(it != edge_map.end());
        #endif
    }
}

void KTruss::compute_support(std::vector<Triangle>& triangles, const Graph& G)
{
    edge_support.resize(G.E.size(), nullptr);
    for(int i = 0; i < (int) edge_support.size(); ++i) //TODO: probably dont need id
    {
        edge_support[i] = new edge({i, 0, -1});
    }
    for(const Triangle& t : triangles)
    {
        for(int i = 0; i < 3; ++i)
        {
            unsigned int v0 = t[i];
            unsigned int v1 = t[(i + 1) % 3];
            auto it = edge_map.find(std::make_pair(v0, v1));
            assert(it != edge_map.end());

            int edge_id = it->second;
            assert(edge_id >= 0 && edge_id < (int) edge_support.size());
            edge* e = edge_support[edge_id];
            assert(e != nullptr);
            e->support++;
        }
    }

    //edge pointers are unique
    #if !NDEBUG
    unordered_map<edge*, int> edge_count;
    for(edge* e : edge_support)
    {
        edge_count[e]++;
    }
    for(auto it = edge_count.begin(); it != edge_count.end(); ++it)
    {
        assert(it->second == 1);
    }
    #endif
}

edge* KTruss::get_edge(unsigned int u, unsigned int v)
{
    auto it = edge_map.find(std::make_pair(u, v));
    assert(it != edge_map.end());
    int edge_id = it->second;
    assert(edge_id >= 0);
    assert(edge_id < (int) edge_support.size());
    return edge_support[edge_id];
}

void KTruss::decrement_support(edge* e)
{
    assert(e != nullptr);
    e->support--;
}

bool is_swap_index_valid(int swap_index, int current_iteration_index)
{
    return swap_index - 1 > current_iteration_index && swap_index > 0;
}

//MARK: Reordering

int KTruss::reorder_edge(edge* e, int current_iteration_index)
{
    assert(e != nullptr);
    assert(current_iteration_index < e->sorted_edges_index);
    assert(current_iteration_index < (int) sorted_edges.size());
    assert(current_iteration_index >= 0);
    
    assert(e == sorted_edges[e->sorted_edges_index]);
    #if !NDEBUG
    if(e->sorted_edges_index - 1 > current_iteration_index)
    {
        edge* left = sorted_edges[e->sorted_edges_index - 1];
        assert(support(e) >= support(left) - 1); // e is in correct support region
    }
    #endif

    int swap_index = e->sorted_edges_index;
    while(is_swap_index_valid(swap_index, current_iteration_index))
    {
        if(support(sorted_edges[swap_index - 1]) < support(e)) // swap_index is last element in new support region of e
        {
            break;
        }
        swap_index--;
    }
    swap_edge(e, swap_index);

    assert(e->sorted_edges_index > current_iteration_index);

    // test order preserved after swap
    #if !NDEBUG
    for(int i = current_iteration_index + 1; i < (int) sorted_edges.size() - 1; ++i)
    {
        assert(support(sorted_edges[i]) <= support(sorted_edges[i + 1]));
    }
    #endif
    return e->sorted_edges_index;
}

void KTruss::swap_edge(edge* e, int swap_index)
{
    assert(e != nullptr);
    assert(swap_index >= 0);
    assert(swap_index < (int) sorted_edges.size());
    assert(e->sorted_edges_index < (int) sorted_edges.size());
    assert(e == sorted_edges[e->sorted_edges_index]);

    edge* e2 = sorted_edges[swap_index];
    assert(e2 != nullptr);
    assert(e2->sorted_edges_index == swap_index);

    assert(support(e) >= support(e2) - 1); // should not disrupt sorted order

    //swap
    sorted_edges[e->sorted_edges_index] = e2;
    sorted_edges[swap_index] = e;
    e2->sorted_edges_index = e->sorted_edges_index;
    e->sorted_edges_index = swap_index;

    assert(sorted_edges[e->sorted_edges_index] == e);
    assert(sorted_edges[e2->sorted_edges_index] == e2);
}

// --------------------------------------------------------------------------------
// --------------------------- MARK: BUCKET SORT ----------------------------------
// --------------------------------------------------------------------------------

BucketSort::BucketSort(vector<edge*>& edge_support)
{
    _size = 0;
    for(size_t i = 0; i < edge_support.size(); ++i)
    {
        insert(edge_support[i]);
    }
}

void BucketSort::insert(edge* e)
{
    assert(e != nullptr);
    if(e->support >= (int) buckets.size())
    {
        buckets.resize(e->support + 1);
    }
    buckets[e->support].push_back(e);
    _size++;
}

vector<edge*> BucketSort::get_sorted_edges()
{
    assert(!buckets.empty());
    vector<edge*> sorted_edges = vector<edge*>(size());
    int i = 0;
    for(vector<edge*>& bucket : buckets)
    {
        for(edge* e : bucket)
        {
            assert((int) sorted_edges.size() > i);
            assert(e != nullptr);
            sorted_edges[i] = e;
            e->sorted_edges_index = i;
            i++;
        }
    }
    return sorted_edges;
}