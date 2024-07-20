#pragma once

#include <vector>
#include <utility>
#include "rn.h"

class Vertex;

/** 
 * Bucketsort for right neighbourhoods, will only work for positive elements
 * @note init: O(n-d), insert: O(1), get_next: O(1), at most O(n) cumulative space+time in main algorithm
 */
class Buckets
{
public:
    /** store the neighbourhood indeces in buckets --> sorted */
    std::vector<std::vector<Vertex*>> buckets;

    // iterator variables
    int current_bucket = -1;
    int current_index = -1;

    int max_bucket = -1; // max bucket with at least one element
    int max_index = -1;

public:
    //default constructor
    Buckets() = default;

    /**
     * put the first N - d right neighbourhoods in buckets (sort)
     */
    Buckets(std::vector<Vertex*>& degeneracy_ordering, std::vector<rn>& right_neighbourhoods, int d);

    /** 
     * @returns neighbourhood index <= the current index, nullptr if no more elements
     * iterate from biggest bucket to smallest
     * 
     * @param rdeg_beq we only need to iterate over right neighbourshoods with rdeg >= d - p
     */
    Vertex* get_next(int rdeg_beq);

    /**
     * @returns current element of the iterator
     * @returns nullptr if there is no right neighbourhood with rdeg >= d - p
     */
    Vertex* get(int rdeg_beq);

    /**
     * @returns (-1, -1) if not found
     */
    std::pair<int, int> get_iterator_of(int vertex_index);

    /**
     * iterator end value to test against in loops
     */
    inline int end() { return -1; };

    /** 
     * reset iterator 
     */
    inline void reset() { current_bucket = max_bucket; current_index = max_index; };

    void print();
};
