#include "tests.h"
#include "graph.h"
#include "debug_utils.h"
#include "colors.h"

#include <cassert>
#include <cstdlib>

using namespace std;

void test_graph_consistency(Graph& G)
{
    cout << CYAN << "----------------- Graph Consistency Test -----------------" << RESET << endl;
    cout << CYAN << "G has N=" << G.N << " and M=" << G.M << RESET << endl;

    test_N_M(G);

    test_vertices(G);

    test_min_max_degree(G);

    test_deg_lists(G);

    //test Vertex neighbors, edges correct
    cout << CYAN << "------------------------ END TEST ------------------------" << RESET << endl;
}

void test_N_M(Graph& G)
{
    size_t N = 0;
    size_t M = 0;
    for(Vertex* v : G.V){
        if (v->status == UNKNOWN)
            N++;
    }
    assert(G.N == N);

    for(Edge* e : G.E){
        assert(e->ends[0].v != nullptr);
        assert(e->ends[1].v != nullptr);
        if(e->ends[0].v->status == UNKNOWN && e->ends[1].v->status == UNKNOWN)
            M++;
    }
    assert(G.M == M);
    print_success("Passed N, M test, N and M are consistent with V and E.");
}

void test_min_max_degree(Graph& G)
{
    int max_deg = -1;
    size_t min_deg = numeric_limits<int>::max();
    for(Vertex* v : G.V){
        if(v->status != UNKNOWN)
            continue;
        if((int) deg(v) > max_deg)
            max_deg = (int) deg(v);
        if(deg(v) < min_deg)
            min_deg = deg(v);
    }
    assert(max_deg == (int) G.max_degree);
    assert(min_deg == G.min_degree);
    print_success("Passed max/min degree test");
}

void test_vertices(Graph& G)
{
    for(Vertex* v : G.V) {
        assert(v->v_idx < G.N);
        for(Vertex* n : v->neighbors){
            assert(n->status == UNKNOWN);
            assert(v->id != n->id);
            assert(n->v_idx < G.N);
        }
    }

    //consistent with edges
    // for(Vertex* v : G.V)
    // {
    //     for(Edge* e : G.E) {
    //         assert(e->ends[0].v == v || e->ends[1].v == v);
    //         bool consistent = false;
    //         Vertex* expected[2] = {e->ends[0].v, e->ends[1].v};
    //         Vertex* found[2];
    //         for (size_t i = 0; i < 2; i++){
    //             found[i] = e->ends[i].v->neighbors[e->ends[i].idx];
    //             consistent |= found[i] == this;
    //         }
    //         assert(consistent);
    //     }
    // }
    
    print_success("Passed vertex test. All neighbours are valid.");

}

void test_deg_lists(Graph& G)
{
    // check that all vertices in deg_lists have consistent degree with index in deg_lists
    for(size_t i = 0; i < G.deg_lists.size(); ++i) {
        for(Vertex* v : G.deg_lists[i]){
            assert(v->status == UNKNOWN);
            assert(deg(v) == i);
        }
    }

    // check that all vertices in graph are in deg_lists
    for(Vertex* v : G.V) {
        if (v->status != UNKNOWN)
            continue;
        assert(deg(v) == v->list_idx);
        assert(G.deg_lists.size() > deg(v));
        assert(!G.deg_lists[deg(v)].empty());
        

        int found_v = -1;
        for(int i = 0; i < (int) G.deg_lists[v->list_idx].size(); ++i)
        {
            Vertex* u = G.deg_lists[v->list_idx][i];
            if(u == v)
            {
                found_v = i;
                break;
            }
        }
        assert(found_v >= 0);
        assert(found_v == ((int) v->deg_idx));
    }

    // check that min_degree and max_degree always point to a valid vertex in deg_lists
    if(G.N > 0)
    {
        assert(G.deg_lists.size() >= G.min_degree && G.deg_lists.size() >= G.max_degree);
        assert(!G.deg_lists[G.min_degree].empty());
        assert(!G.deg_lists[G.max_degree].empty());
    }
    print_success("Passed deg_lists test");
}

