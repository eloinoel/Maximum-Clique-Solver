#include "solve_via_vc.h"
#include "solve.h" // vertex cover solver
#include "degeneracy_ordering.h"
#include "upper_bounds.h"
#include "lower_bounds.h"
#include "k_core.h"
#include "graph.h"
#include "debug_utils.h"
#include "tests.h"
#include "colors.h"

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
    auto LB_maximum_clique = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoods);
    clique_LB = (int) LB_maximum_clique.size();
    

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
    if(clique_LB == clique_UB)
    {
        //copy Vertex* vector to solution string vector
        maximum_clique = std::vector<std::string>();
        for(size_t i = 0; i < LB_maximum_clique.size(); ++i)
        {
            Vertex* v = LB_maximum_clique[i];
            size_t v_id = LB_maximum_clique[i]->id;
            maximum_clique.push_back(G.name_table[v_id]);
        }
        return clique_LB;
    }

    for(int p = 0; p < clique_UB; p++) //TODO: check if this can be reduced
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

//MARK: SOLVER FOR P
bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        int vc_size = 0;
        Graph complement;
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        Vertex* vi = D[i];
        size_t vc_UB = (right_neighbourhoods[vi->id].size() + p - d) + 1;
        if(right_neighbourhoods[vi->id].size() > 0)
        {
            complement = get_complement_subgraph(G, right_neighbourhoods[vi->id]);
            complement.UB = vc_UB; //bound search tree
            vc_size = solve(complement);
            complement.delete_all();
        }

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        if(vc_size == (int) (right_neighbourhoods[vi->id].size() + p - d))
        {
            if(right_neighbourhoods[vi->id].empty())
            {
                maximum_clique.push_back(G.name_table[vi->id]);
            }
            else
            {
                extract_maximum_clique_solution(complement, vi);
            }

            return true;
        }
    }

    // if we can't find mc from right neighbourhoods, take remaining vertices in ordering
    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
    auto Vf = get_remaining_set();
    int vc_size = 0;
    Graph complement;
    size_t vc_UB = p;
    if(Vf.size() > 0)
    {
        complement = get_complement_subgraph(G, Vf);
        complement.UB = vc_UB; //bound vc search tree
        vc_size = solve(complement);
        complement.delete_all();
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

//MARK: EXTRACT SOLUTION
void SolverViaVC::extract_maximum_clique_solution(Graph& complementGraph, Vertex* o)
{

    //TODO: remove debug
    /*std::cout << RED << "---------- complement graph VC ----------" << RESET << std::endl;
    for(size_t i = 0; i < complementGraph.partial.size(); ++i)
    {
        std::cout << RED << complementGraph.name_table[complementGraph.partial[i]->id] << RESET << std::endl;
    }*/
    //maximum_clique = std::vector<std::string>();
    vector<Vertex*> max_clique;
    //fill maximum_clique
    size_t i = 0;
    auto mark_sol = complementGraph.new_timestamp();
    for(Vertex* v : complementGraph.partial)
        v->marked = mark_sol;
    
    for(Vertex* v : complementGraph.V){
        if(v->marked != mark_sol)
            max_clique.push_back(v);
    }
    /*for(Vertex* v : complementGraph.V)
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
    }*/

    if(o)
        max_clique.push_back(o);

    for(Vertex* v : max_clique){
        maximum_clique.push_back(complementGraph.name_table[v->id]);
    }

}


//----------------------- TESTING FUNCTIONS -----------------------

Vertex* SolverViaVC::get_vertex_by_name(Graph& G, std::string name)
{
    for(Vertex* v : G.V)
    {
        if(G.name_table[v->id] == name)
        {
            return v;
        }
    }
    return nullptr;
}

std::vector<Vertex*> SolverViaVC::convert_vertex_list(Graph& G, std::vector<std::string> names)
{
    std::vector<Vertex*> vertices = std::vector<Vertex*>(names.size());
    for(size_t i = 0; i < names.size(); ++i)
    {
        vertices[i] = get_vertex_by_name(G, names[i]);
    }
    return vertices;
}

bool SolverViaVC::is_clique(std::vector<Vertex*> vertices)
{
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        for(size_t j = 0; j < vertices.size(); ++j)
        {
            if(i != j && !vertices[i]->adjacent_to(vertices[j]))
            {
                return false;
            }
        }
    }
    return true;
}
