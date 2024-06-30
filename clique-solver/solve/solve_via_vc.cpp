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

int SolverViaVC::solve_via_vc(Graph& G)
{
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
        cout << "Graph with N=" << initial_N << " and M=" << initial_M << " has degeneracy d=" << d << endl;
    #endif

    //check lower and upper bounds
    clique_UB = degeneracy_UB(d);
    auto LB_maximum_clique = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoods);
    clique_LB = (int) LB_maximum_clique.size();

    #if DEBUG
        cout << "Computed LB=" << clique_LB << ", UB=" << clique_UB << endl;
    #endif

    if(clique_LB == clique_UB)
    {
        //copy Vertex* vector to solution string vector
        maximum_clique = vector<string>();
        for(size_t i = 0; i < LB_maximum_clique.size(); ++i)
        {
            Vertex* v = LB_maximum_clique[i];
            size_t v_id = LB_maximum_clique[i]->id;
            maximum_clique.push_back(G.name_table[v_id]);
        }
        return clique_LB;
    }

    for(int p = 0; p < clique_UB; p++)
    {
        if(solve_via_vc_for_p(G, p, clique_LB))
        {
            return clique_UB - p;
        }
    }
    return clique_LB;
}

Graph get_complement_subgraph(Graph & G, vector<Vertex*>& induced_set, int LB)
{
    G.set_restore();
    G.MM_induced_subgraph(induced_set);
    Graph complement = G.complementary_graph(G);
    G.restore();
    return complement;
}

//MARK: SOLVER FOR P
bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p, int LB)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        int vc_size = 0;
        bool found = false;
        Graph complement;
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        Vertex* vi = D[i];
        size_t r_neighbourhood_size = right_neighbourhoods[vi->id].size();
        size_t vc_UB = r_neighbourhood_size + p - d + 1;
        if(r_neighbourhood_size > 0)
        {
            complement = get_complement_subgraph(G, right_neighbourhoods[vi->id], LB);
            complement.UB = vc_UB; //bound search tree
            auto res = solve_limitUB(complement, vc_UB);
            found = res.first;
            vc_size = res.second;
        }

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        if( found && vc_size == (int) (r_neighbourhood_size + p - d))
        {
            if(right_neighbourhoods[vi->id].empty())
            {
                maximum_clique.push_back(G.name_table[vi->id]);
            }
            else
            {
                extract_maximum_clique_solution(complement, vi);
            }
            complement.delete_all();
            return true;
        }
        complement.delete_all();
    }

    // if we can't find mc from right neighbourhoods, take remaining vertices in ordering
    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
    auto Vf = get_remaining_set();
    int vc_size = 0;
    Graph complement;
    size_t vc_UB = p;
    if(Vf.size() > 0)
    {
        complement = get_complement_subgraph(G, Vf, LB);
        complement.UB = vc_UB; //bound vc search tree
        vc_size = solve(complement);
    }

    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    if(vc_size == (int) p - 1)
    {
        if(Vf.size() > 0)
        {
            extract_maximum_clique_solution(complement);
        }
        complement.delete_all();
        return true;
    }
    complement.delete_all();
    return false;
}

vector<Vertex*> SolverViaVC::get_candidate_set(size_t p)
{
    vector<Vertex*> D = vector<Vertex*>();
    for(size_t i = 0; i < degeneracy_ordering.size() - d; ++i)
    {
        Vertex* vi = degeneracy_ordering[i];
        if(right_neighbourhoods[vi->id].size() >= d - p)
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

    if(o)
        max_clique.push_back(o);

    for(Vertex* v : max_clique){
        maximum_clique.push_back(complementGraph.name_table[v->id]);
    }

}


//----------------------- TESTING FUNCTIONS -----------------------

Vertex* SolverViaVC::get_vertex_by_name(Graph& G, string name)
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

vector<Vertex*> SolverViaVC::convert_vertex_list(Graph& G, vector<string> names)
{
    vector<Vertex*> vertices = vector<Vertex*>(names.size());
    for(size_t i = 0; i < names.size(); ++i)
    {
        vertices[i] = get_vertex_by_name(G, names[i]);
    }
    return vertices;
}

bool SolverViaVC::is_clique(vector<Vertex*> vertices)
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
