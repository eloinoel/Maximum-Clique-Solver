#include "k_truss.h"
#include "graph.h"

using namespace std;

KTruss::KTruss(const Graph& G, std::vector<Vertex*>& degeneracy_ordering)
{
    H = make_shared<Graph>(G.shallow_copy());
    init_edge_map(*(H.get()));

    //block scope to free variable memory when not needed anymore
    {
        std::vector<Triangle> triangles = compute_triangles(*(H.get()), degeneracy_ordering);
        //compute support for each edge, i.e. how many triangles it is a part of
        compute_support(triangles, *(H.get()));
    }
    {
        //sort all edges in ascending order of their support
        BucketSort b = BucketSort(edge_support);
        sorted_edges = b.get_sorted_edges();
    }
}

KTruss::~KTruss()
{
    for(edge* e : edge_support)
    {
        assert(e != nullptr);
        delete e;
    }
}

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

        for(int j = 0; j < u->neighbors.size(); ++j)
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
        if(k_classes.size() <= k)
        {
            k_classes.resize(k+1);
        }
        k_classes[k].push_back(e->id);

        //remove edge from graph
        H->remove_edge(H->E[e->id]);
    }
}


std::vector<Triangle> KTruss::compute_triangles(Graph& G, std::vector<Vertex*>& degeneracy_ordering)
{
    //TODO:
    return std::vector<Triangle>();
}

void KTruss::init_edge_map(Graph& G)
{
    for(size_t i = 0; i < G.E.size(); ++i)
    {
        unsigned int v0 = G.E[i]->ends[0].v->id;
        unsigned int v1 = G.E[i]->ends[1].v->id;
        edge_map[std::make_pair(v0, v1)] = i;

        #if !NDEBUG
        auto it = edge_map.find(std::make_pair(v1, v0));
        assert(it != edge_map.end());
        it = edge_map.find(std::make_pair(v0, v1));
        assert(it != edge_map.end());
        #endif
    }
}

void KTruss::compute_support(std::vector<Triangle>& triangles, const Graph& G)
{
    edge_support.resize(G.E.size(), new edge({-1, 0}));
    for(int i = 0; i < (int) edge_support.size(); ++i) //TODO: probably dont need id
    {
        edge_support[i]->id = i;
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
            edge* e = edge_support[edge_id];
            assert(e != nullptr);
            e->support++;
        }
    }
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
    return swap_index > current_iteration_index && swap_index > 0;
}

int KTruss::reorder_edge(edge* e, int current_iteration_index)
{
    assert(e != nullptr);
    assert(current_iteration_index < e->sorted_edges_index);
    assert(current_iteration_index < (int) sorted_edges.size());
    assert(current_iteration_index >= 0);
    
    assert(e == sorted_edges[e->sorted_edges_index]);
    if(e->sorted_edges_index - 1 > current_iteration_index)
    {
        assert(support(e) >= support(sorted_edges[e->sorted_edges_index - 1])); // e is in correct support region
    }

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

    assert(support(e) == support(e2)); // should not disrupt sorted order

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