#include "bucket_sort.h"
#include "graph.h"
#include "solve_via_vc.h"
#include "colors.h"

using namespace std;

//MARK: BUCKETS
Buckets::Buckets(std::vector<Vertex*>& degeneracy_ordering, std::vector<rn>& right_neighbourhoods, int d)
{
    size_t N = degeneracy_ordering.size();
    // right-neighbourhoods have at most d vertices
    buckets = std::vector<std::vector<int>>(d + 1);

    for(size_t i = 0; i < N - d; ++i)
    {
        int vertex_id = degeneracy_ordering[i]->id;
        int rdeg = right_neighbourhoods[vertex_id].neigh.size();

        assert(buckets.size() > rdeg); //Should always be the case due to degeneracy definition
        buckets[rdeg].push_back(vertex_id);

        // update iterator variables
        if(rdeg > max_bucket)
        {
            max_bucket = rdeg;
            max_index = buckets[rdeg].size() - 1;
        }
    }
    current_bucket = max_bucket;
    current_index = max_index;
}

void Buckets::insert(int vertex_index, int neighbourhood_size)
{
    assert(neighbourhood_size < buckets.size()); // d + 1 < buckets.size() per definition

    buckets[neighbourhood_size].push_back(vertex_index);

    // update iterator variables
    if(neighbourhood_size > max_bucket)
    {
        max_bucket = neighbourhood_size;
        max_index = buckets[neighbourhood_size].size() - 1;
    }
}


int Buckets::get_next(int rdeg_beq)
{
    if(current_bucket == -1 || current_index == -1 || current_bucket < rdeg_beq)
        return -1;
    
    assert(buckets.size() > current_bucket);
    assert(buckets[current_bucket].size() > current_index);

    // if there are more elements in the current bucket
    current_index--;
    if(current_index >= 0) {
        return buckets[current_bucket][current_index];
    }

    // else find next non-empty bucket
    current_bucket--;
    while(current_bucket >= 0 && current_bucket >= rdeg_beq && buckets[current_bucket].empty())
    {
        current_bucket--;
    }

    // return an element if there are any left
    if(current_bucket == -1 || current_bucket < rdeg_beq)
    {
        return -1;
    }
    current_index = buckets[current_bucket].size() - 1;

    assert(buckets.size() > current_bucket);
    assert(buckets[current_bucket].size() > current_index);
    return buckets[current_bucket][current_index];
}

int Buckets::get(int rdeg_beq)
{
    if(current_bucket == -1 || current_index == -1 || current_bucket < rdeg_beq)
        return -1;

    assert(buckets.size() > current_bucket);
    assert(buckets[current_bucket].size() > current_index);
    return buckets[current_bucket][current_index];
}

void Buckets::print()
{
    for(size_t i = 0; i < buckets.size(); ++i)
    {
        if(buckets[i].empty())
            continue;
        cout << "Bucket " << CYAN << i << RESET << ": ";
        for(size_t j = 0; j < buckets[i].size(); ++j)
        {
            cout << GREEN << buckets[i][j] << RESET << " ";
        }
        cout << endl;
    }
}
