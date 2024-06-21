#include "solve_via_vc.h"
#include "solve.h" // vertex cover solver
#include "degeneracy_ordering.h"
#include "upper_bounds.h"
#include "lower_bounds.h"
#include "k_core.h"
#include "graph.h"
#include "debug_utils.h"
#include "tests.h"

using namespace std;

//TODO: store the MC solution for output
//TODO: store complement graphs for each right neighbourhood
//TODO: maybe optimise subgraph/complement graph construction
//TODO: use better data reductions
//TODO: make better use of lower and upper Bounds, eg. heuristical lower bound
int SolverViaVC::solve_via_vc(Graph& G)
{
    #if !NDEBUG
        print_success("Starting MC solver using VC solver");
        size_t initial_N = G.N;
        size_t initial_M = G.M;
    #endif

    if(G.N == 0){ return 0; }

    //compute degeneracy ordering and degeneracy d of G
    auto result = degeneracy_ordering_rN(G);
    degeneracy_ordering = move(result.first);
    right_neighbourhoods = move(result.second);
    d = degeneracy(degeneracy_ordering, right_neighbourhoods);

    #if !NDEBUG
        cout << "Graph with N=" << initial_N << " and M=" << initial_M << " has degeneracy d=" << d << std::endl;
    #endif

    //check lower and upper bounds
    cliqueUB = degeneracy_UB(d);
    cliqueLB = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoods);

    #if !NDEBUG
        cout << "Computed LB=" << cliqueLB << ", UB=" << cliqueUB << std::endl;
    #endif

    //data reduction
    apply_k_core(G, cliqueLB);

    test_graph_consistency(G); //TODO: remove debug

    //TODO: Possibly need to exclude vertices from ordering which were excluded by k-core

    #if !NDEBUG
        cout << "Applied Data Reductions. new_N=" << G.N << ", new_M=" << G.M << std::endl;
    #endif

    for(int p = cliqueUB; p >= cliqueLB; p--)
    {
        if(solve_via_vc_for_p(G, p))
        {
            return cliqueUB - p;
        }
    }
    return cliqueLB;
}

bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        G.set_restore();
        G.MM_induced_subgraph(right_neighbourhoods[i]);
        Graph complement = G.complementary_graph(G);
        test_graph_consistency(complement); //TODO: remove debug
        G.restore();

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        int vc_size = solve(complement);
        if(vc_size == (int) (right_neighbourhoods[i].size() + p - d))
        {
            return true;
        }
    }

    // if we can't find mc from right neighbourhoods, take remaining vertices in ordering
    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
    G.set_restore();
    auto Vf = get_remaining_set();
    G.MM_induced_subgraph(Vf);
    Graph complement = G.complementary_graph(G);
    G.restore();

    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    int vc_size = solve(complement);
    if(vc_size == (int) p - 1)
    {
        return true;
    }

    return false;
}

vector<Vertex*> SolverViaVC::get_candidate_set(size_t p)
{
    vector<Vertex*> D = vector<Vertex*>();
    for(size_t i = 0; i < degeneracy_ordering.size() - d; ++i)
    {
        if(right_neighbourhoods[i].size() >= d - p)
        {
            D.push_back(degeneracy_ordering[i]);
        }
    }
    return D;
}

vector<Vertex*> SolverViaVC::get_remaining_set()
{
    vector<Vertex*> R = vector<Vertex*>();
    for(size_t i = degeneracy_ordering.size() - d; i < degeneracy_ordering.size(); ++i)
    {
        R.push_back(degeneracy_ordering[i]);
    }
    return R;
}
