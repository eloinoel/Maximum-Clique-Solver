#include "clique_cover.h"


typedef struct{
    size_t remaining = 0;
    size_t clique_size = 0;
}clique_info;

int basic_clique_cover(Graph& G){
    G.new_timestamp();
    unsigned long long start = G.timestamp;

    vector<clique_info> cliques;
    for(size_t i = 1; i < G.deg_lists.size();i++){
        for(Vertex* v : G.deg_lists[i]){
            G.new_timestamp();
            auto v_iter = G.timestamp; // timestep of current iteration, speeds up checks
            // join best existing clique
            size_t best_idx = -1;

            for(Vertex* n : v->neighbors){
                if(n->marked >= start){
                    clique_info& clique = cliques[n->data.clique_data.clique_idx];

                    if(n->marked != v_iter){
                        clique.remaining = clique.clique_size;
                        n->marked = v_iter;
                    }

                    clique.remaining--;

                    if(clique.remaining == 0 && (best_idx == (size_t) -1 || clique.clique_size > cliques[best_idx].clique_size)){
                        best_idx = n->data.clique_data.clique_idx;
                    }

                }
            }
            if(best_idx != (size_t) -1){
                v->marked = v_iter;
                v->data.clique_data.clique_idx = best_idx;
                cliques[best_idx].clique_size++;
                continue;
            }
            // if none exist, create a new one
            if(v->marked < start){
                v->marked = v_iter;
                v->data.clique_data.clique_idx = cliques.size();
                cliques.push_back({1,1});
            }
        }
    }

    assert(cliques.size() <= G.N);

    return G.N - cliques.size() + G.sol_size;
}


int block_clique_cover(Graph& G){
    size_t j = 0;
    for(size_t i = 1; i < G.deg_lists.size();i++){
        for(Vertex* v : G.deg_lists[i]){
            v->data.block.clique_size = 1;
            v->data.block.clique_idx  = j++;
            v->data.block.next        = nullptr;
        }
    }
    size_t need = 0;
    for(size_t i = 1; i < G.deg_lists.size();i++){ 
        for(Vertex* v : G.deg_lists[i]){
            if(v->data.block.next != nullptr)
                continue;
            G.new_timestamp();
            auto v_iter = G.timestamp;

            Vertex* best = nullptr;
            int best_idx = numeric_limits<int>::max();
            int best_size = -1;

            for(Vertex* n : v->neighbors){
                n->marked = v_iter;
            }

            for(Vertex* n : v->neighbors){
                if(n->data.block.clique_size != 1){
                    continue;
                }else{
                    while((n->data.block.next != nullptr) && (n->data.block.next->marked == v_iter)){
                        n = n->data.block.next;
                    }
                    
                    if(n->data.block.next != nullptr){
                        continue;
                    }
                    else{
                        //if(static_cast<int>(n->data.block.clique_size) > best_size){
                        if(n->data.block.clique_idx < best_idx){
                            best_idx = n->data.block.clique_idx;
                            best = n;
                            best_size = n->data.block.clique_size;
                        }
                    }
                }
            }

            if(best != nullptr){
                best->data.block.next = v;
                v->data.block.clique_size = (best_size + 1);
                need++;
            }
        }
    }

    
    return need + G.sol_size;
}

#define MAX_ITER      50
#define MAX_ITER_FULL 10
#define EARLY_STOP     1

// so compiler can remove unused options
#define USE_JUMP 0
#define USE_REV 0
#define USE_SORT 0
constexpr double P_REV = 0.2; // probability of doing a reversal of the permutation
constexpr double P_JUMP = 0.4; // probability of jump
constexpr double P_SORT = 0.2; // probability of largest-first sort

// only looks for immediate improvement at adjacent blocks, fast-ish even at 50-100 iterations
int iterated_greedy_clique(Graph& G){
    vector<Vertex*> blocks;
    for(size_t i = 1; i < G.deg_lists.size();i++){
        for(Vertex* v : G.deg_lists[i]){
            if(v->data.block.clique_size == 1) //start of a clique
                blocks.push_back(v);
        }
    }

    int best_lb = 0;
    for(size_t i = 0; i < MAX_ITER; i++){
        // permutate blocks
        shuffle(blocks.begin(), blocks.end(), G.gen);
        std::vector<size_t> deleted_blocks;

        //try to entend cliques using permutation
        size_t j = 0;
        while(j + 1 < blocks.size()){

            Vertex* next_block  = blocks[j+1];
            size_t p            = 0;

            while(true){
                G.new_timestamp();
                auto iter = G.timestamp;

                for(auto& n : next_block->neighbors){
                    n->marked = iter;
                }

                for(Vertex* b = blocks[j]; b != nullptr; b = b->data.block.next){
                    if(b->marked != iter){
                        goto fail;
                    }
                }
                p         += 1;
                next_block = next_block->data.block.next;
                if(!next_block)
                    break;
            }

            fail:
            if(p == 0){
                j++;
                continue;
            }

            Vertex* b_end = blocks[j]; // end of the clique we can add p members to

            while(b_end->data.block.next != nullptr)
                b_end = b_end->data.block.next;
            
            Vertex* b_p = blocks[j+1]; // start of clique j + 1 that we can take p members of

            //induces clique, we can add add first p members of j+1 to j
            for(size_t k = 0; k < p; k++){
                b_end->data.block.next      = b_p;
                b_p->data.block.clique_size = b_end->data.block.clique_size + 1;

                b_end                       = b_end->data.block.next;  // always current end of clique j
                b_p                         = b_p->data.block.next;    // always p-th member of clique j + 1
            }
            // block j + 1 has remaining members -> adjust their clique size, or mark the block for deletion (Don't delete it yet to avoid biasing the randomness)
            if(b_p != nullptr){
                b_end->data.block.next = nullptr; // set the end of clique j
                Vertex* b_rest = b_p;             // manage rest of clique j + 1
                size_t c                    = 1;
                
                while(b_rest){
                    b_rest->data.block.clique_size = c++;
                    b_rest = b_rest->data.block.next;
                }
                blocks[j+1] = b_p;               // set new start of clique j + 1
            }
            else{
                deleted_blocks.push_back(j+1);
                j++;
            }
            j++;
        }

        for(auto rt = deleted_blocks.rbegin(); rt != deleted_blocks.rend(); rt++){
            swap(blocks.back(), blocks[*rt]);
            blocks.pop_back();
        }
        deleted_blocks.clear();
        int need = G.sol_size + G.N - blocks.size();
        best_lb = max(best_lb, need);
        if(best_lb >= G.UB && EARLY_STOP) return best_lb;
    }

    return best_lb;

}

// a little bit better than iterated_greedy_clique but fairly slow
int iterated_greedy_clique_full_permutes(Graph& G){
    int best_lb = 0;
    size_t best_time = 0;
    uniform_real_distribution<double> dist(0, 1);
    for(size_t ii = 0; ii < MAX_ITER_FULL; ii++){
        if(best_lb >= G.UB && EARLY_STOP)
            return best_lb;
        vector<Vertex*> blocks;
        size_t max_block = 0;
        for(size_t i = 1; i < G.deg_lists.size();i++){
            for(Vertex* v : G.deg_lists[i]){
                max_block = max(max_block,  v->data.block.clique_size);

                if(v->data.block.clique_size == 1){ //start of a clique
                    blocks.push_back(v);
                }
            }
        }
        vector<Vertex*> order(G.N);
        
        double random_choice = dist(G.gen);
        if(random_choice < P_REV && USE_REV){
            size_t k = 0;
            vector<Vertex*> reversersal_buffer(max_block);
            for(Vertex* b : blocks){
                size_t i = 0;
                do{
                    reversersal_buffer[i] = b;
                    b = b->data.block.next;
                    i++;
                }while(b);

                for(size_t j = i; j-- > 0;){
                    
                    order[G.N - (1 + k++)] = reversersal_buffer[j];
                }
                
            }
        }else if(random_choice < P_SORT && USE_SORT){
            vector<vector<Vertex*>> sizes(max_block);

            for(Vertex* b : blocks){
                size_t i = 0;
                Vertex* cur = b;
                do{
                    cur = cur->data.block.next;
                    i++;
                }while(cur);
                sizes[i-1].push_back(b);
            }
            size_t k = 0;
            for(size_t i = sizes.size(); i-- > 0;){
                auto& size_class = sizes[i];
                for(Vertex* b : size_class){
                    do{
                        order[k] = b;
                        b = b->data.block.next;
                        k++;
                    }while(b);
                }
            }

        }else if (random_choice < P_JUMP && USE_JUMP){
            std::uniform_int_distribution<int> idist(0, blocks.size()-1);
            int jump = idist(G.gen);

            vector<Vertex*> jumped(blocks.size());
            for(size_t i = 0; i < jump; i++){
                jumped[i+1] = blocks[i];
            }
            for(size_t i = jump+1; i < blocks.size(); i++){
                jumped[i] = blocks[i];
            }
            jumped[0] = blocks[jump];

            size_t i = 0;
            for(Vertex* b : jumped){
                do{
                    order[i] = b;
                    b = b->data.block.next;
                    i++;
                }while(b);
            }

        }else{
            shuffle(blocks.begin(), blocks.end(), G.gen);
            size_t i = 0;
            for(Vertex* b : blocks){
                do{
                    order[i] = b;
                    b = b->data.block.next;
                    i++;
                }while(b);
            }
        }


        for(size_t j = 0; j < order.size();j++){
            Vertex* v                 = order[j];
            v->data.block.clique_idx  = j;
            v->data.block.clique_size = 1;
            v->data.block.next        = nullptr;
        }
        
        size_t need = 0;
        for(size_t i = 0; i < order.size();i++){ 
            Vertex* v = order[i];
            if(v->data.block.next != nullptr)
                continue;

            G.new_timestamp();
            auto v_iter   = G.timestamp;

            Vertex* best  = nullptr;
            int best_size = -1;
            int best_idx  = numeric_limits<int>::max();

            for(Vertex* n : v->neighbors){
                n->marked = v_iter;
            }

            for(Vertex* n : v->neighbors){
                if(n->data.block.clique_size != 1){
                    continue;
                }else{
                    while((n->data.block.next != nullptr) && (n->data.block.next->marked == v_iter)){
                        n = n->data.block.next;
                    }
                    
                    if(n->data.block.next != nullptr){
                        continue;
                    }
                    else{
                        if(n->data.block.clique_idx < best_idx){
                            best      = n;
                            best_size = n->data.block.clique_size;
                            best_idx  = n->data.block.clique_idx;
                        }
                    }
                }
            }

            if(best != nullptr){
                best->data.block.next     = v;
                v->data.block.clique_idx  = best_idx; 
                v->data.block.clique_size = (best_size + 1);
                need++;
            }
        }
        //if(need + G.sol_size > best_lb){
        //    best_time = ii;
            
            //cout << "LB: " << need + G.sol_size << " at " << best_time << ", check if monotic " << need + G.sol_size << " ITER:" << ii <<  "\n";
        //}
        best_lb = max(best_lb, static_cast<int>(need + G.sol_size));
        //cout << "LB: " << best_lb << " at " << best_time << ", check if monotic " << need + G.sol_size << " ITER:" << ii <<  "\n";
    }
        
    return best_lb;
}

// still doesn't work great around local optima
int iterated_greedy_clique_full_permutes_class_index(Graph& G){
    int best_lb = 0;
    size_t best_time = 0;
    uniform_real_distribution<double> dist(0, 1);
    for(size_t ii = 0; ii < MAX_ITER_FULL; ii++){
        if(best_lb >= G.UB && EARLY_STOP)
            return best_lb;
        vector<Vertex*> blocks;
        size_t max_block = 0;
        for(size_t i = 1; i < G.deg_lists.size();i++){
            for(Vertex* v : G.deg_lists[i]){
                max_block = max(max_block,  v->data.block.clique_size);

                if(v->data.block.clique_size == 1){ //start of a clique
                    blocks.push_back(v);
                }
            }
        }
        vector<Vertex*> order(G.N);
        
        double random_choice = dist(G.gen);
        if(random_choice < P_REV && USE_REV){
            size_t k = 0;
            vector<Vertex*> reversersal_buffer(max_block);
            for(Vertex* b : blocks){
                size_t i = 0;
                do{
                    reversersal_buffer[i] = b;
                    b = b->data.block.next;
                    i++;
                }while(b);

                for(size_t j = i; j-- > 0;){
                    
                    order[G.N - (1 + k++)] = reversersal_buffer[j];
                }
                
            }
        }else if(random_choice < P_SORT && USE_SORT){
            vector<vector<Vertex*>> sizes(max_block);

            for(Vertex* b : blocks){
                size_t i = 0;
                Vertex* cur = b;
                do{
                    cur = cur->data.block.next;
                    i++;
                }while(cur);
                sizes[i-1].push_back(b);
            }
            size_t k = 0;
            for(size_t i = sizes.size(); i-- > 0;){
                auto& size_class = sizes[i];
                for(Vertex* b : size_class){
                    do{
                        order[k] = b;
                        b = b->data.block.next;
                        k++;
                    }while(b);
                }
            }

        }else if (random_choice < P_JUMP && USE_JUMP){
            std::uniform_int_distribution<int> idist(0, blocks.size()-1);
            int jump = idist(G.gen);

            vector<Vertex*> jumped(blocks.size());
            for(size_t i = 0; i < jump; i++){
                jumped[i+1] = blocks[i];
            }
            for(size_t i = jump+1; i < blocks.size(); i++){
                jumped[i] = blocks[i];
            }
            jumped[0] = blocks[jump];

            size_t i = 0;
            for(Vertex* b : jumped){
                do{
                    order[i] = b;
                    b = b->data.block.next;
                    i++;
                }while(b);
            }

        }else{
            shuffle(blocks.begin(), blocks.end(), G.gen);
            size_t i = 0;
            size_t j = 0;
            for(Vertex* b : blocks){
                do{
                    order[i] = b;
                    b->data.block.clique_idx = j;
                    b = b->data.block.next;
                    i++;
                }while(b);
                j++; 
            }
        }


        for(size_t j = 0; j < order.size();j++){
            Vertex* v                 = order[j];
            //v->data.block.clique_idx  = j;
            v->data.block.clique_size = 1;
            v->data.block.next        = nullptr;
        }
        
        size_t need = 0;
        for(size_t i = 0; i < order.size();i++){ 
            Vertex* v = order[i];
            if(v->data.block.next != nullptr)
                continue;

            G.new_timestamp();
            auto v_iter   = G.timestamp;

            Vertex* best  = nullptr;
            int best_size = -1;
            int best_idx  = numeric_limits<int>::max();

            for(Vertex* n : v->neighbors){
                n->marked = v_iter;
            }

            for(Vertex* n : v->neighbors){
                if(n->data.block.clique_size != 1){
                    continue;
                }else{
                    while((n->data.block.next != nullptr) && (n->data.block.next->marked == v_iter)){
                        n = n->data.block.next;
                    }
                    
                    if(n->data.block.next != nullptr){
                        continue;
                    }
                    else{
                        if(n->data.block.clique_idx < best_idx){
                            best      = n;
                            best_size = n->data.block.clique_size;
                            best_idx  = n->data.block.clique_idx;
                        }
                    }
                }
            }

            if(best != nullptr){
                best->data.block.next     = v;
                v->data.block.clique_idx  = best_idx; 
                v->data.block.clique_size = (best_size + 1);
                need++;
            }
        }
        //if(need + G.sol_size > best_lb){
        //    best_time = ii;
            
            //cout << "LB: " << need + G.sol_size << " at " << best_time << ", check if monotic " << need + G.sol_size << " ITER:" << ii <<  "\n";
        //}
        best_lb = max(best_lb, static_cast<int>(need + G.sol_size));
        //cout << "LB: " << best_lb << " at " << best_time << ", check if monotic " << need + G.sol_size << " ITER:" << ii <<  "\n";
    }
        
    return best_lb;
}
