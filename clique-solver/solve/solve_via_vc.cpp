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
#include "heuristic/AMTS.h"

using namespace std;

//TODO: binary search for p
//TODO: maybe optimise subgraph/complement graph construction
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
    std::vector<Vertex*> lb_vertices = degeneracy_ordering_LB(degeneracy_ordering, right_neighbourhoodsT);
    initial_LB_solution = get_str_clique(lb_vertices, G);
    LB_clique_vertices = &initial_LB_solution; // store to ptr so we don't have to copy in update_LB
    clique_LB = (int) LB_clique_vertices->size();

    vector<string> amts_clique;
    if(USE_AMTS_MILLISECONDS > 0)
    {
        amts_clique = compute_amts_LB(G);
        if((int) amts_clique.size() > clique_LB)
        {
            //cout << "amts LB better " << amts_clique.size() << " > " << clique_LB << endl;
            LB_clique_vertices = &amts_clique;
            clique_LB = (int) amts_clique.size();
        }
    }

    #if DEBUG
        cout << "Computed LB=" << clique_LB << ", UB=" << clique_UB << std::endl;
    #endif

    // terminate early
    if(clique_LB == clique_UB)
    {
        maximum_clique = *LB_clique_vertices;
        return clique_LB;
    }

    
    if(BINARY_SEARCH)
    {
        return binary_search_solve(G);
    }
    else
    {
        // linear search: solve for increasing values of p (clique core gap)
        return linear_solve(G);
    }

    //cout << "after for loop, returning " << maximum_clique.size() << endl;
    return maximum_clique.size();
}

//MARK: LINEAR & BINARY 

int SolverViaVC::linear_solve(Graph& G)
{
    int clique_size = 0;
    for(int p = 0; p <= clique_UB - clique_LB; p++)
    {
        if(solve_via_vc_for_p_with_sorting(G, p))
        {
            return clique_size = clique_UB - p;
        }
    }
    return maximum_clique.size();
}

vector<string> SolverViaVC::compute_amts_LB(Graph& G)
{
    AMTS amts = AMTS(G);
    vector<string> heuristic_clique = amts.find_best(clique_LB, USE_AMTS_MILLISECONDS, G);
    return heuristic_clique;
}

bool check_found_maximum_clique(int p, int clique_LB, int clique_UB)
{
    return p + clique_LB >= clique_UB;
}

int SolverViaVC::binary_search_solve(Graph& G)
{
    int p = 0;
    while(p <= clique_UB - clique_LB)
    {
        int p_ub = clique_UB - clique_LB; // can change due to clique_LB getting updated in solver
        int binary_search_p = (p + p_ub) / 2;
        //cout << "bs_p: " << GREEN << binary_search_p << RESET << ", p: " << p << ", p_ub: " << p_ub << endl;
        bool found_clique = solve_via_vc_for_p_with_sorting(G, binary_search_p);
        if(found_clique && (int) maximum_clique.size() > clique_LB)
        {
            //cout << "Found clique of size=" << maximum_clique.size() << endl;
            update_LB(maximum_clique);
        }
        else 
        {
            p++;
            if(p + clique_LB >= clique_UB)
            {
                //cout << "return clique_UB - p : " << clique_UB - p << endl;
                maximum_clique = *LB_clique_vertices;
                return clique_UB - p;
            }
            int next_binary_search_p = (p + p_ub) / 2;
            if(next_binary_search_p == binary_search_p)
            {
                p++;
            }
        }

        if(p + clique_LB >= clique_UB)
        {
            //cout << "return clique_UB - p : " << clique_UB - p << endl;
            maximum_clique = *LB_clique_vertices;
            return clique_UB - p;
        }

    }
    return maximum_clique.size();
}

//MARK: SOLVER SORTING
bool SolverViaVC::solve_via_vc_for_p_with_sorting(Graph &G, size_t p)
{
    // start with remaining set as detailed section "Algorithm Implementation" of the paper
    //cout << "Solving for p=" << p << endl;
    if(remaining_set_solution.first == N_INF) { // if not solved yet
        // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1
        auto Vf = get_remaining_set();
        remaining_set_solution.first = 0;
        Graph complement;
        //size_t vc_UB = p;
        if(Vf.size() > 0) 
        {
            //cout << "Vf size: " << Vf.size() << ", LB: " << clique_LB << endl;
            if((int) Vf.size() <= clique_LB) //don't need to solve
            {
                //vc size is <= 0 to have at least all vertices of Vf in clique
                remaining_set_solution.first = Vf.size() - clique_LB; 
                (remaining_set_solution.second) = *LB_clique_vertices;
            }
            else
            {
                complement = get_complement_subgraph(G, Vf);
                //complement.UB = vc_UB; //bound vc search tree
                remaining_set_solution.first = solve(complement);
                remaining_set_solution.second = extract_maximum_clique_solution(complement);
                update_LB(remaining_set_solution.second);
            }
        }
        complement.delete_all();
    }
    //if G[Vf] has a vertex cover of size qf := p − 1, return true
    if(remaining_set_solution.first == (int) p - 1)
    {
        //cout << "Found solution in remaining set" << endl;
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
        //size_t vc_UB = r_neighbourhood_size + p - d + 1;

        if(r_neighbourhood_size > 0)
        {
            //if not already solved
            if(right_neighbourhoods[cur_vertex_id].vc_size == -1)
            {
                int r_neighbourhood_ub = r_neighbourhood_size + 1;
                if(r_neighbourhood_ub <= clique_LB) // LB already better solution
                {
                    right_neighbourhoods[cur_vertex_id].vc_size = r_neighbourhood_size + 1 - clique_LB;
                    right_neighbourhoods[cur_vertex_id].sol = *LB_clique_vertices;
                }
                else
                {
                    Graph complement = get_complement_subgraph(G, right_neighbourhoods[cur_vertex_id].neigh);
                    //complement.UB = vc_UB; //bound search tree
                    int vc_size = solve(complement);
                    right_neighbourhoods[cur_vertex_id].vc_size = vc_size;
                    right_neighbourhoods[cur_vertex_id].sol = extract_maximum_clique_solution_from_rn(complement);
                    right_neighbourhoods[cur_vertex_id].sol.push_back(complement.name_table[cur_vertex_id]);
                    complement.delete_all();
                    update_LB(right_neighbourhoods[cur_vertex_id].sol);
                }
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
            //cout << "Found solution in candidate set for vertex " << cur_vertex_id << endl;
            //complement.delete_all();
            return true;
        }
        cur_vertex_id = D.get_next(d - p);
    }
    return false;
}

//MARK: SOLVER NO SORTING
bool SolverViaVC::solve_via_vc_for_p(Graph &G, size_t p)
{
    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}
    vector<Vertex*> D = get_candidate_set(p);

    for(size_t i = 0; i < D.size(); ++i)
    {
        // a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
        Vertex* vi = D[i];
        size_t r_neighbourhood_size = right_neighbourhoods[vi->id].neigh.size();
        //size_t vc_UB = r_neighbourhood_size + p - d + 1;
        if(r_neighbourhood_size > 0)
        {
            //if not already solved
             if(right_neighbourhoods[vi->id].vc_size == -1)
             {
                Graph complement = get_complement_subgraph(G, right_neighbourhoods[vi->id].neigh);
                //complement.UB = vc_UB; //bound search tree
                int vc_size = solve(complement);
                right_neighbourhoods[vi->id].vc_size = vc_size;
                right_neighbourhoods[vi->id].sol = extract_maximum_clique_solution_from_rn(complement);
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
    //size_t vc_UB = p;
    if(Vf.size() > 0)
    {
        complement = get_complement_subgraph(G, Vf);
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


//MARK: UTILITY METHODS

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
        assert(sorted_candidate_set.max_index == (int) sorted_candidate_set.buckets[max_bucket].size() - 1);

        // test iterator functions
        for(size_t i = 0; i < degeneracy_ordering.size() - d; ++i)
        {
            int v_id = degeneracy_ordering[i]->id;
            if((int) right_neighbourhoods[v_id].neigh.size() < d)
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

Graph SolverViaVC::get_complement_subgraph(Graph & G, std::vector<Vertex*>& induced_set)
{
    G.set_restore();
    G.MM_induced_subgraph(induced_set);
    Graph complement = G.complementary_graph(G);
    G.restore();
    return complement;
}

//MARK: EXTRACT SOLUTION

std::vector<std::string> SolverViaVC::extract_maximum_clique_solution_from_rn(Graph& complementGraph){
    vector<string> max_clique_str;
    //fill maximum_clique
    auto mark_sol = complementGraph.new_timestamp();
    for(Vertex* v : complementGraph.partial)
        v->marked = mark_sol;
    
    for(Vertex* v : complementGraph.V){
        if(v->marked != mark_sol)
            max_clique_str.push_back((complementGraph.name_table[v->id]));
    }

    return max_clique_str;
}


vector<string> SolverViaVC::extract_maximum_clique_solution(Graph& complementGraph, Vertex* o)
{
    vector<Vertex*> max_clique;
    //fill maximum_clique
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

vector<string> SolverViaVC::get_str_clique(std::vector<Vertex*>& vertices, Graph& G)
{
    std::vector<string> str_clique = std::vector<std::string>();
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        size_t v_id = vertices[i]->id;
        str_clique.push_back(G.name_table[v_id]);
    }
    return str_clique;
}

void SolverViaVC::update_LB(std::vector<std::string>& solution_candidate)
{
    if((int) solution_candidate.size() > clique_LB)
    {
        clique_LB = solution_candidate.size();
        LB_clique_vertices = &solution_candidate;
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