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
        Buckets b = Buckets(edge_support);
        sorted_edges = b.get_sorted_edges();
    }
}

KTruss::~KTruss()
{
    for(edge* e : edge_support)
    {
        delete e;
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
            edge_support[edge_id]->support++;
        }
    }
}

// --------------------------------------------------------------------------------
// --------------------------- MARK: BUCKET SORT ----------------------------------
// --------------------------------------------------------------------------------

Buckets::Buckets(vector<edge*>& edge_support)
{
    _size = 0;
    for(size_t i = 0; i < edge_support.size(); ++i)
    {
        insert(edge_support[i]);
    }
}

void Buckets::insert(edge* e)
{
    assert(e != nullptr);
    if(e->support >= (int) buckets.size())
    {
        buckets.resize(e->support + 1);
    }
    buckets[e->support].push_back(e);
    _size++;
}

vector<edge*> Buckets::get_sorted_edges()
{
    assert(!buckets.empty());
    vector<edge*> sorted_edges;
    for(vector<edge*>& bucket : buckets)
    {
        for(edge* e : bucket)
        {
            sorted_edges.push_back(e);
        }
    }
}