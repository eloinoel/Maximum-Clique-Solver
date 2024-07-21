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
        //print_triangles(triangles);
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

    //initialize support region indices
    if(!sorted_edges.empty())
    {
        support_region_index.resize(sorted_edges.back()->support + 1, -1);
        int current_support_region = -1;
        for(int i = 0; i < (int) sorted_edges.size(); ++i)
        {
            edge* current_edge = sorted_edges[i];
            assert(current_edge != nullptr);
            //cout << current_edge->support << " " << current_support_region << endl;
            while(current_edge->support > current_support_region)
            {
                assert((int) support_region_index.size() > current_edge->support);
                assert((int)support_region_index.size() > current_support_region + 1);
                support_region_index[current_support_region + 1] = i;
                current_support_region++;
            }
        }
    }

    //print_support(PrintVertices::Names);
    //print_support_region_indices();
    //cout << "finished setting up support region indices" << endl;
    //cout << "H.M = " << H->M << endl;


    #if !NDEBUG
    for(int i = 0; i < (int) support_region_index.size(); ++i)
    {
        assert(support_region_index[i] >= 0);
        assert(support_region_index[i] < (int) sorted_edges.size());
        assert(sorted_edges[support_region_index[i]]->support >= i);
        //is first
        if(support_region_index[i] > 0)
        {
            assert(sorted_edges[support_region_index[i] - 1]->support < i);
        }
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
    #if !NDEBUG
        removed_edges.clear();
    #endif
    k_classes = std::vector<std::vector<Edge*>>();
    int k = 0;
    //print_support(PrintVertices::Names);
    for(size_t i = 0; i < sorted_edges.size(); ++i)
    {
        edge* e = sorted_edges[i]; //edge with lowest support
        assert(e != nullptr);
        assert(e->support >= 0);
        assert(e->id >= 0);

        //cout << i << "-th edge: " << e->id << ", support: " << e->support << endl;

        // update support_region_index when current_iteration_index advances
        support_region_index[support(e)] = i + 1;
        if(support(e) - 1 >= 0)
        {
            support_region_index[support(e) - 1] = support_region_index[support(e)];
        }
        
        //TODO: remove debug
        //cout << "-------" << endl;
        //cout << "k = " << k << "\n" << i << "th ";
        //print_edge(e, PrintVertices::Names);

        while(e->support > k)
        {
            k++;
        }

        //we want u to have the smaller degree for efficiency
        assert(e->graph_edge != nullptr);
        Vertex* u = H->E[e->graph_edge->idx]->ends[0].v;
        Vertex* v = H->E[e->graph_edge->idx]->ends[1].v;
        if (deg(u) > deg(v))
        {
            std::swap(u, v);
        }

        // cout << "-------------------------------------------------------" << endl;
        // cout << i << "-th edge: " << e->id  << ", " << H->name_table[u->id] << "--" << H->name_table[v->id] << 
        // ", k=" << k << endl;
        // print_support(PrintVertices::Names);

        for(int j = 0; j < (int) u->neighbors.size(); ++j)
        {
            Vertex* w = u->neighbors[j];
            // found triangle --> update supports and remove edge
            if(is_edge(v->id, w->id))
            {
                edge* u_w = get_edge(u->id, w->id);
                edge* v_w = get_edge(v->id, w->id);

                //cout << "triangle: " << H->name_table[u->id] << " - " << H->name_table[v->id] << " - " << H->name_table[w->id] << endl;
                //update support and reorder according to new support
                decrement_support(u_w);
                reorder_edge(u_w, i); //TODO: reordering could be done in O(1) with maps for index buckets

                //print_support(PrintVertices::Names);

                decrement_support(v_w);
                reorder_edge(v_w, i);

                //print_support(PrintVertices::Names);
            }
        }
        //add edge to k-class
        for(int i = k_classes.size(); i <= k; ++i) //TODO: check whether resize is more efficient
        {
            k_classes.push_back(std::vector<Edge*>());
        }
        k_classes[k].push_back(e->graph_edge);

        //remove edge from graph
        #if !NDEBUG
            // verify that not already deleted
            assert(removed_edges.find(e->id) == removed_edges.end());
            removed_edges[e->id] = true;
        #endif

        //cout << "remove edge " << H->name_table[u->id] << " - " << H->name_table[v->id] << endl;
        remove_edge_from_graph(e);
    }
    computed_k_classes = true;
}

size_t KTruss::reduce(Graph& G, int lower_clique_bound)
{
    #if !NDEBUG
        removed_edges.clear();
    #endif

    size_t num_deleted_edges = 0;
    // deduce from k_classes
    if(computed_k_classes)
    {
        for(int i = 0; i < (int) k_classes.size() && lower_clique_bound - 2; ++i)
        {
            for(Edge* graph_edge : k_classes[i])
            {
                #if !NDEBUG
                // verify that not already deleted
                assert(removed_edges.find(graph_edge->idx) == removed_edges.end());
                removed_edges[graph_edge->idx] = true;
                #endif
                (void)G.remove_edge(graph_edge);
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
                assert(removed_edges.find(e->graph_edge->idx) == removed_edges.end());
                removed_edges[e->graph_edge->idx] = true;
                #endif
                (void)G.remove_edge(e->graph_edge);
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
        return ((int) k_classes.size() - 1) + 2;
    }
    else
    {
        compute_k_classes();
        return ((int) k_classes.size() - 1) + 2;
    }
}

// MARK: Prints

void KTruss::print_edge(edge* e, PrintVertices print_vertices)
{
    assert(e != nullptr);
    Vertex* v0 = e->graph_edge->ends[0].v;
    Vertex* v1 = e->graph_edge->ends[1].v;
    assert(v0 != nullptr);
    assert(v1 != nullptr);
    switch(print_vertices)
    {
        case PrintVertices::Names:
            cout << "edge(" << RED << e->id << RESET << ") " << "sup=" << CYAN << e->support << RESET << ", " << GREEN << H->name_table[v0->id] << " - " << H->name_table[v1->id] << RESET << endl;
            break;
        case PrintVertices::IDs:
            cout << "edge(" << RED << e->id << RESET << ") " << "sup=" << CYAN << e->support << RESET << ", " << GREEN << v0->id << " - " << v1->id << RESET << endl;
            break;
        default:
            break;
    }
    

}

void KTruss::print_triangles(std::vector<Triangle>& triangles)
{
    cout << "----------------------- Triangles -------------------------" << endl;
    for(Triangle t : triangles)
    {
        cout << "triangle: " << H->name_table[t[0]] << " - " << H->name_table[t[1]] << " - " << H->name_table[t[2]] << endl;
    }
}

void KTruss::print_support_region_indices()
{
    for(int i = 0; i < (int) support_region_index.size(); ++i)
    {
        cout << "sup(" << GREEN << i << RESET << ")" << " --> " << RED << support_region_index[i] << RESET << endl;
    }
}

void KTruss::print_support(PrintVertices print_vertices)
{
    int i = 0;
    cout << "-------------------------------------------------------" << endl;
    for(edge* e : sorted_edges)
    {
        assert(e != nullptr);
        switch(print_vertices)
        {
            case PrintVertices::Names:
                cout << "[" << i << "] edge(" << RED << e->id << RESET<< ") " << GREEN << H->name_table[e->graph_edge->ends[0].v->id] << " - " << H->name_table[e->graph_edge->ends[1].v->id] << RESET << " support: " << CYAN << e->support << RESET << endl;
                break;
            case PrintVertices::IDs:
                cout << "[" << i << "] edge(" << RED << e->id << RESET<< ") " << GREEN << e->graph_edge->ends[0].v->id << " - " << e->graph_edge->ends[1].v->id << RESET << " support: " << CYAN << e->support << RESET << endl;
                break;
            default:
                break;
        }
        i++;
    }
}

void KTruss::print_k_classes(PrintVertices print_vertices)
{
    cout << "----------------------- k-classes -------------------------" << endl;
    for(int i = 0; i < (int) k_classes.size(); ++i)
    {
        cout << PURPLE << "k = " << i << RESET << endl;
        for(Edge* e : k_classes[i])
        {
            switch(print_vertices)
            {
                case PrintVertices::Names:
                    cout << "edge(" << RED << e->idx << RESET << ") " << GREEN << H->name_table[e->ends[0].v->id] << " - " << H->name_table[e->ends[1].v->id] << RESET << endl;
                    break;
                case PrintVertices::IDs:
                    cout << "edge(" << RED << e->idx << RESET << ") " << GREEN << e->ends[0].v->id << " - " << e->ends[1].v->id << RESET << endl;
                    break;
                default:
                    break;
            }
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
        assert(G.E[i] != nullptr);
        edge_support[i] = new edge({G.E[i], i, 0, -1});
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

    int original_index = e->sorted_edges_index;
    int original_e_support = support(e) + 1;
    int swap_index = original_index;

    swap_index = support_region_index[original_e_support];
    
    //print_support(PrintVertices::Names);
    //print_support_region_indices();
    //cout << "current_iteration_index: " << current_iteration_index << endl;
    //cout << "swap index: " << swap_index << ", original index: " << original_index << endl;
    
    assert(swap_index > current_iteration_index);
    
    //TODO: update support_region_indices after swap, is this sufficient?
    //to the right

    // inefficent garbage that is in the worst case O(n)
    // while(is_swap_index_valid(swap_index, current_iteration_index))
    // {
    //     if(support(sorted_edges[swap_index - 1]) < original_e_support) // swap_index is last element in new support region of e
    //     {
    //         break;
    //     }
    //     swap_index--;
    // }

    swap_edge(e, swap_index);

    if(swap_index + 1 < (int) sorted_edges.size() && support(sorted_edges[swap_index + 1]) >= original_e_support)
    {
        assert((int) support_region_index.size() > support(sorted_edges[swap_index + 1]));
        support_region_index[original_e_support] = swap_index + 1;
        //cout << "support_region_index[" << original_e_support << "] = " << swap_index + 1 << endl;
    }

    if(swap_index == original_index)
    {
        support_region_index[original_e_support] = swap_index + 1;
    }

    // if no elements to the left, update support_region_index of current support
    if(is_swap_index_valid(swap_index, current_iteration_index) && support(sorted_edges[swap_index - 1]) < support(e))
    {
        support_region_index[support(e)] = swap_index;
        // if((int) sorted_edges.size() > swap_index + 1)
        // {
        //     support_region_index[original_e_support] = swap_index + 1;
        // }
    }

    assert(e->sorted_edges_index > current_iteration_index);

    // test order preserved after swap
    #if !NDEBUG
    for(int i = current_iteration_index + 1; i < (int) sorted_edges.size() - 1; ++i)
    {
        edge* e1 = sorted_edges[i];
        edge* e2 = sorted_edges[i + 1];
        if(!(support(e1) <= support(e2)))
        {
            //print_support();
            //print_support_region_indices();
        }
        assert(support(e1) <= support(e2));
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

void KTruss::remove_edge_from_graph(edge* e)
{
    assert(e != nullptr);
    assert(e->graph_edge != nullptr);
    (void)H->remove_edge(e->graph_edge);
    #if !NDEBUG
    auto it = edge_map.find(std::make_pair(e->graph_edge->ends[0].v->id, e->graph_edge->ends[1].v->id));
    assert(it != edge_map.end());
    #endif
    edge_map.erase(std::make_pair(e->graph_edge->ends[0].v->id, e->graph_edge->ends[1].v->id));
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