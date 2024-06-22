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
//TODO: sort first n-d vertices in ordering by right degree
//TODO: maybe optimise subgraph/complement graph construction
//TODO: use better data reductions
//TODO: make better use of lower and upper Bounds, eg. heuristical lower bound
int SolverViaVC::solve_via_vc(Graph& G)
{
    //solution_complement_graph = Graph();
    #if DEBUG
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

    #if DEBUG
        cout << "Graph with N=" << initial_N << " and M=" << initial_M << " has degeneracy d=" << d << std::endl;
    #endif

    //check lower and upper bounds
    clique_UB = degeneracy_UB(d);
    clique_LB = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoods);

    #if DEBUG
        cout << "Computed LB=" << clique_LB << ", UB=" << clique_UB << std::endl;
    #endif

    //data reduction
    //apply_k_core(G, clique_LB);

    //test_graph_consistency(G); //TODO: remove debug

    //TODO: Possibly need to exclude vertices from ordering which were excluded by k-core or compute max k-core before ordering

    #if DEBUG
        cout << "Applied Data Reductions. new_N=" << G.N << ", new_M=" << G.M << std::endl;
    #endif

    // for(int p = clique_UB; p >= clique_LB; p--)
    // {
    //     if(solve_via_vc_for_p(G, p))
    //     {
    //         return clique_UB - p;
    //     }
    // }
    for(int p = 0; p < clique_UB - clique_LB; p++)
    {
        if(solve_via_vc_for_p(G, p))
        {
            return clique_UB - p;
        }
    }
    return clique_LB;
}

Graph get_complement_subgraph(Graph & G, std::vector<Vertex*>& induced_set)
{
    G.set_restore();
    G.MM_induced_subgraph(induced_set);
    Graph complement = G.complementary_graph(G);
    //test_graph_consistency(complement); //TODO: remove debug
    G.restore();
    return complement;
}

bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        int vc_size = 0;
        Graph complement;
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        if(right_neighbourhoods[i].size() > 0)
        {
            complement = get_complement_subgraph(G, right_neighbourhoods[i]);
            vc_size = solve(complement);
        }

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        if(vc_size == (int) (right_neighbourhoods[i].size() + p - d))
        {
            if(right_neighbourhoods[i].empty())
            {
                maximum_clique.push_back(G.name_table[D[i]->id]);
            }
            else
            {
                extract_maximum_clique_solution(complement, G.name_table[D[i]->id]);
            }

            return true;
        }
    }

    // if we can't find mc from right neighbourhoods, take remaining vertices in ordering
    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
    auto Vf = get_remaining_set();
    int vc_size = 0;
    Graph complement;
    if(Vf.size() > 0)
    {
        complement = get_complement_subgraph(G, Vf);
        vc_size = solve(complement);
    }

    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    if(vc_size == (int) p - 1)
    {
        if(Vf.size() > 0)
        {
            extract_maximum_clique_solution(complement);
        }
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

void SolverViaVC::extract_maximum_clique_solution(Graph& complementGraph, string ordering_vertex_name)
{
    assert(complementGraph.sol_size == (int) complementGraph.partial.size());
    maximum_clique = std::vector<std::string>();

    //fill maximum_clique
    size_t i = 0;
    for(Vertex* v : complementGraph.V)
    {
        bool v_in_vc = false;
        for(Vertex* u : complementGraph.partial)
        {
            if(v == u)
            {
                v_in_vc = true;
                break;
            }
        }
        if(!v_in_vc)
        {
            assert(v->id < complementGraph.name_table.size());
            //assert(i < maximum_clique.size());
            //maximum_clique[i] = complementGraph.name_table[v->id];
            maximum_clique.push_back(complementGraph.name_table[v->id]);
            ++i;
        }
    }

    if(ordering_vertex_name != "")
        maximum_clique.push_back(ordering_vertex_name);
}
