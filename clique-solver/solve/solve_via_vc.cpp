#include "solve_via_vc.h"
#include "solve.h" // vertex cover solver
#include "./../orderings/degeneracy_ordering.h"
#include "upper_bounds.h"
#include "lower_bounds.h"
#include "k_core.h"
#include "graph.h"
#include "debug_utils.h"
#include "tests.h"
#include "colors.h"

using namespace std;


//TODO: sort first n-d vertices in ordering by right degree
//TODO: binary search for p
//TODO: maybe optimise subgraph/complement graph construction
//TODO: use better data reductions
//TODO: make better use of lower and upper Bounds, eg. heuristical lower bound
//TODO: store complement graphs for each right neighbourhood
int SolverViaVC::solve_via_vc(Graph& G)
{
    #if DEBUG
        print_success("Starting MC solver using VC solver");
        size_t initial_N = G.N;
        size_t initial_M = G.M;
    #endif

    if(G.N == 0){ return 0; }

    // compute degeneracy ordering and degeneracy d of G
    auto result = degeneracy_ordering_rN(G);
    degeneracy_ordering = move(result.first);
    std::vector<std::vector<Vertex*>> right_neighbourhoodsT = move(result.second);
    d = degeneracy(degeneracy_ordering, right_neighbourhoodsT);

    #if DEBUG
        cout << "Graph with N=" << initial_N << " and M=" << initial_M << " has degeneracy d=" << d << std::endl;
    #endif

    // copy right neighbourhoods to rn struct //TODO: unnecessary copy
    for(auto r : right_neighbourhoodsT){
        vector<Vertex*> x = r;
        right_neighbourhoods.push_back({move(x), -1, {}});
    }
    //check lower and upper bounds
    clique_UB = degeneracy_UB(d);
    auto LB_maximum_clique = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoodsT);
    clique_LB = (int) LB_maximum_clique.size();

    #if DEBUG
        cout << "Computed LB=" << clique_LB << ", UB=" << clique_UB << std::endl;
    #endif

    // terminate early
    if(clique_LB == clique_UB)
    {
        set_maximum_clique(LB_maximum_clique, G);
        return clique_LB;
    }

    // solve for increasing values of p (clique core gap)
    for(int p = 0; p < clique_UB; p++) //TODO: check if this can be reduced
    {
        if(solve_via_vc_for_p_with_sorting(G, p, clique_LB))
        {
            return clique_UB - p;
        }
    }
    return clique_LB;
}

//MARK: SOLVER FOR P
bool SolverViaVC::solve_via_vc_for_p_with_sorting(Graph &G, size_t p, int LB)
{
    // start with remaining set as detailed section "Algorithm Implementation" of the paper
    if(remaining_set_solution.first == -1) { // if not solved yet
        // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
        auto Vf = get_remaining_set();
        remaining_set_solution.first = 0;
        Graph complement;
        size_t vc_UB = p;
        if(Vf.size() > 0) 
        {
            complement = get_complement_subgraph(G, Vf, LB);
            //complement.UB = vc_UB; //bound vc search tree
            remaining_set_solution.first = solve(complement);
            remaining_set_solution.second = extract_maximum_clique_solution(complement);
        }
        complement.delete_all();
    }
    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    if(remaining_set_solution.first == (int) p - 1)
    {
        maximum_clique = remaining_set_solution.second;
        //complement.delete_all();
        return true;
    }

    // get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}, sorted descending by their right degree
    Buckets D = get_sorted_candidate_set();
    int cur_vertex_id = D.get(d - p);

    // iterate through all right neighbourhoods where rdeg(vi) >= d - p
    while(cur_vertex_id != D.end())
    {
        size_t r_neighbourhood_size = right_neighbourhoods[cur_vertex_id].neigh.size();
        size_t vc_UB = r_neighbourhood_size + p - d + 1;

        if(r_neighbourhood_size > 0)
        {
            //if not already solved
             if(right_neighbourhoods[cur_vertex_id].vc_size == -1)
             {
                Graph complement = get_complement_subgraph(G, right_neighbourhoods[cur_vertex_id].neigh, LB);
                //complement.UB = vc_UB; //bound search tree
                int vc_size = solve(complement);
                right_neighbourhoods[cur_vertex_id].vc_size = vc_size;
                right_neighbourhoods[cur_vertex_id].sol = extract_maximum_clique_solution_from_rn(right_neighbourhoods[cur_vertex_id], complement);
                right_neighbourhoods[cur_vertex_id].sol.push_back(complement.name_table[cur_vertex_id]);
                complement.delete_all();
            }
        }

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        int target = r_neighbourhood_size + p - d;
        int current_vc_size = right_neighbourhoods[cur_vertex_id].vc_size;
        if(current_vc_size == target){
            if(right_neighbourhoods[cur_vertex_id].neigh.empty())
            {
                maximum_clique.push_back(G.name_table[cur_vertex_id]);
            }
            else
            {
                maximum_clique = right_neighbourhoods[cur_vertex_id].sol;
            }
            //complement.delete_all();
            return true;
        }
        cur_vertex_id = D.get_next(d - p);
    }
    return false;
}

bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p, int LB)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        Vertex* vi = D[i];
        size_t r_neighbourhood_size = right_neighbourhoods[vi->id].neigh.size();
        size_t vc_UB = r_neighbourhood_size + p - d + 1;
        if(r_neighbourhood_size > 0)
        {
            //if not already solved
             if(right_neighbourhoods[vi->id].vc_size == -1)
             {
                Graph complement = get_complement_subgraph(G, right_neighbourhoods[vi->id].neigh, LB);
                //complement.UB = vc_UB; //bound search tree
                int vc_size = solve(complement);
                right_neighbourhoods[vi->id].vc_size = vc_size;
                right_neighbourhoods[vi->id].sol = extract_maximum_clique_solution_from_rn(right_neighbourhoods[vi->id], complement);
                right_neighbourhoods[vi->id].sol.push_back(complement.name_table[vi->id]);
                complement.delete_all();
            }
        }

        //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true
        int target = r_neighbourhood_size + p - d;
        int current_vc_size = right_neighbourhoods[vi->id].vc_size;
        if(current_vc_size == target){
            if(right_neighbourhoods[vi->id].neigh.empty())
            {
                maximum_clique.push_back(G.name_table[vi->id]);
            }
            else
            {
                maximum_clique = right_neighbourhoods[vi->id].sol;
            }
            //complement.delete_all();
            return true;
        }
    }

    // if we can't find mc from right neighbourhoods, take remaining vertices in ordering
    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
    auto Vf = get_remaining_set();
    int vc_size_r = 0;
    Graph complement;
    size_t vc_UB = p;
    if(Vf.size() > 0)
    {
        complement = get_complement_subgraph(G, Vf, LB);
        //complement.UB = vc_UB; //bound vc search tree
        vc_size_r = solve(complement);
    }

    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    if(vc_size_r == (int) p - 1)
    {
        if(Vf.size() > 0)
        {
            extract_maximum_clique_solution(complement);
        }
        //complement.delete_all();
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
        if(right_neighbourhoods[vi->id].neigh.size() >= d - p)
        {
            D.push_back(degeneracy_ordering[i]);
        }
    }
    return D;
}

Buckets& SolverViaVC::get_sorted_candidate_set()
{
    if(sorted_candidate_set_initialised)
    {
        sorted_candidate_set.reset();
        return sorted_candidate_set;
    }
    sorted_candidate_set = Buckets(degeneracy_ordering, right_neighbourhoods, d);
    sorted_candidate_set_initialised = true;

    // test consistency of Buckets
    #if !NDEBUG
        //cout << "Testing buckets consistency" << endl;
        // every vertex is in the correct bucket
        for(size_t i = 0; i < sorted_candidate_set.buckets.size(); ++i)
        {
            for(size_t j = 0; j < sorted_candidate_set.buckets[i].size(); ++j)
            {
                assert(right_neighbourhoods[sorted_candidate_set.buckets[i][j]].neigh.size() == i);
            }
        }

        // every vertex vi of the degeneracy ordering  is in the buckets, where vi <= n - d
        for(size_t i = 0; i < degeneracy_ordering.size() - d; ++i)
        {
            auto [bucket, index] = sorted_candidate_set.get_iterator_of(degeneracy_ordering[i]->id);
            assert(bucket != sorted_candidate_set.end());
            assert(index != sorted_candidate_set.end());
        }
        
        // test iterator variables
        int max_bucket = -1;
        for(size_t i = 0; i < sorted_candidate_set.buckets.size(); ++i)
        {
            if(sorted_candidate_set.buckets[i].size() > 0)
            {
                max_bucket = i;
            }
        }
        assert(sorted_candidate_set.max_bucket == max_bucket);
        assert(sorted_candidate_set.max_index == sorted_candidate_set.buckets[max_bucket].size() - 1);

        // test iterator functions
        for(size_t i = 0; i < degeneracy_ordering.size() - d; ++i)
        {
            int v_id = degeneracy_ordering[i]->id;
            if(right_neighbourhoods[v_id].neigh.size() < d)
                continue;
            int current = sorted_candidate_set.get(d);
            bool found = false;
            while(current != sorted_candidate_set.end())
            {
                if(current == v_id)
                {
                    found = true;
                    break;
                }
                current = sorted_candidate_set.get_next(d);
            }
            if(!found)
            {
                auto [bucket, index] = sorted_candidate_set.get_iterator_of(v_id);
                cout << "Vertex " << v_id << " not found with iterator, rdeg(v): " << right_neighbourhoods[v_id].neigh.size()
                << ", cur_bucket: " << sorted_candidate_set.current_bucket 
                << ", cur_index: " << sorted_candidate_set.current_index 
                << ", correct bucket: " << bucket << ", correct index: " << index
                << endl;
                assert(found);
            }
            sorted_candidate_set.reset();
        }
    #endif

    return sorted_candidate_set;
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

Graph SolverViaVC::get_complement_subgraph(Graph & G, std::vector<Vertex*>& induced_set, int LB)
{
    G.set_restore();
    G.MM_induced_subgraph(induced_set);
    Graph complement = G.complementary_graph(G);
    G.restore();
    return complement;
}

std::vector<std::string> SolverViaVC::extract_maximum_clique_solution_from_rn(rn& right, Graph& complementGraph){
    vector<string> max_clique_str;
    //fill maximum_clique
    size_t i = 0;
    auto mark_sol = complementGraph.new_timestamp();
    for(Vertex* v : complementGraph.partial)
        v->marked = mark_sol;
    
    for(Vertex* v : complementGraph.V){
        if(v->marked != mark_sol)
            max_clique_str.push_back((complementGraph.name_table[v->id]));
    }

    return max_clique_str;
}

//MARK: EXTRACT SOLUTION
vector<string> SolverViaVC::extract_maximum_clique_solution(Graph& complementGraph, Vertex* o)
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

    vector<string> max_str_clique;
    for(Vertex* v : max_clique){
        max_str_clique.push_back(complementGraph.name_table[v->id]);
    }
    return max_str_clique;
}

void SolverViaVC::set_maximum_clique(std::vector<Vertex*>& vertices, Graph& G)
{
    maximum_clique = std::vector<std::string>();
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        size_t v_id = vertices[i]->id;
        maximum_clique.push_back(G.name_table[v_id]);
    }
    return;
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