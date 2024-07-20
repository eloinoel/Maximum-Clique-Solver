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
    buckets = std::vector<std::vector<Vertex*>>(d + 1);

    for(size_t i = 0; i < N - d; ++i)
    {
        Vertex* v = degeneracy_ordering[i];
        int rdeg = right_neighbourhoods[v->id].neigh.size();

        assert((int)buckets.size() > rdeg); //Should always be the case due to degeneracy definition
        buckets[rdeg].push_back(v);

        // update iterator variables
        if(rdeg >= max_bucket)
        {
            max_bucket = rdeg;
            max_index = buckets[rdeg].size() - 1;
        }
    }
    current_bucket = max_bucket;
    current_index = max_index;
}


Vertex* Buckets::get_next(int rdeg_beq)
{
    if(current_bucket == -1 || current_index == -1 || current_bucket < rdeg_beq)
    {
        return nullptr;
    }
    
    assert((int)buckets.size() > current_bucket);
    assert((int)buckets[current_bucket].size() > current_index);

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
        return nullptr;
    }
    current_index = buckets[current_bucket].size() - 1;
    

    assert((int)buckets.size() > current_bucket);
    assert((int)buckets[current_bucket].size() > current_index);
    return buckets[current_bucket][current_index];
}

Vertex* Buckets::get(int rdeg_beq)
{
    if(current_bucket == -1 || current_index == -1 || current_bucket < rdeg_beq)
        return nullptr;

    assert((int)buckets.size() > current_bucket);
    assert((int)buckets[current_bucket].size() > current_index);
    return buckets[current_bucket][current_index];
}

std::pair<int, int> Buckets::get_iterator_of(int vertex_index) 
{
    for(size_t i = 0; i < buckets.size(); ++i)
    {
        for(size_t j = 0; j < buckets[i].size(); ++j)
        {
            if((int) buckets[i][j]->id == vertex_index)
            {
                return std::make_pair(i, j);
            }
        }
    }
    return std::make_pair(-1, -1);
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
