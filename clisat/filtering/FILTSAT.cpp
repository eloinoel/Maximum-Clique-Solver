#include "FILTSAT.h"
#include <algorithm> // temp for sort...

#define PSAT_USED 33123 // arbitrary constant, used to see if something (wrongly) keeps it later



list<Vertex*> FILTSAT::filter(Graph& G, bool& cut){
    // TODO: remove need to sort here
    // sort(branchset.begin(), branchset.end(), [](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});
    
    vector<int> filtered;

    for(size_t i = soft_clauses.size(); i-- > 0;){
        
        soft_clauses[i].active = false;

        
        if(detect_conflict_iset(G, soft_clauses[i].literals, filtered)){
            //cout << "fathomed node\n";
            cut = true;
            return {};
        }

        for(auto f_iter = filtered.rbegin(); f_iter != filtered.rend(); f_iter++){
            int f = *f_iter;
            swap(soft_clauses[i].literals[f], soft_clauses[i].literals.back());
            soft_clauses[i].literals.pop_back();
            //cout << "FILTSAT FILTERED\n";
        }

        filtered.clear();

        soft_clauses[i].active = true;
    }

    cut = false;
    return output_pruned_to_list();
}

// b_iset = the independent set to check for failed literals
bool FILTSAT::detect_conflict_iset(Graph& G, vector<Vertex*>& b_iset, vector<int>& to_filter){

    auto old_clauses = soft_clauses; // copy for later, not worth it to undo all the changes
    for(size_t k = 0; k < b_iset.size(); k++){
        Vertex* lit = b_iset[k];
        auto adj_lit = G.new_timestamp();
        for(Vertex* n : lit->neighbors){
            n->marked = adj_lit;
        }

        vector<int> units;
        for(size_t i = 0; i < soft_clauses.size(); i++){
            if(!soft_clauses[i].active)
                continue;
            auto& clause = soft_clauses[i].literals;

            size_t j = 0;
            while(j < clause.size()){
                Vertex* literal = clause[j];
                if(literal->marked != adj_lit){
                    swap(clause[j], clause.back());
                    clause.pop_back();
                }else{
                    j++;
                }
            }

            if(clause.empty()){
                to_filter.push_back(k);
                goto success;
            }

            if(clause.size() == 1)
                units.push_back(i);
        }

        /* try unit propagation with new clauses */
        if(units.empty()){
           soft_clauses = old_clauses;
           return false;
        }

        if(propagate_units(G, units)){
            to_filter.push_back(k);
        }
        else{
            return false;
        }
        success:
        soft_clauses = old_clauses;

        continue;

    }

    /* the independent set b_iset causes a conflict
     * since this is the filter case, we can cut the branch
     */
    soft_clauses = old_clauses;
    return true;
}

bool FILTSAT::propagate_units(Graph& G, vector<int>& units){
    for(size_t i = 0; i < units.size(); i++){
        assert(soft_clauses[units[i]].literals.size() == 1);
        //if(!soft_clauses[units[i]].active)
        //    continue; not needed?
        soft_clauses[units[i]].active = false; // this clause is fulfilled now
        Vertex* unit = soft_clauses[units[i]].literals[0];

        //swap(soft_clauses[units[i]].literals, soft_clauses.back().literals);
        //soft_clauses.pop_back();

        auto adj_unit = G.new_timestamp();

        for(Vertex* n : unit->neighbors){
            n->marked = adj_unit;
        }

        for(size_t k = 0; k < soft_clauses.size(); k++){
            if(!soft_clauses[k].active)
                continue;
            auto& clause = soft_clauses[k].literals;
            //auto& slack  = soft_clauses[k].slack;

            size_t j = 0;
            bool was_unit = (clause.size() == 1);
            while(j < clause.size()){
                Vertex* literal = clause[j];
                if(literal->marked != adj_unit){
                    swap(clause[j], clause.back());
                    clause.pop_back();
                }else{
                    j++;
                }
            }

            if(clause.empty()){
                return true;
            }

            if(clause.size() == 1 && !was_unit)
                units.push_back(k);
        }
        
    }

    return false;
}


list<Vertex*> FILTSAT::output_pruned_to_list(){
    list<Vertex*> pruned;
    soft_clauses.pop_back(); // this is the branchset
    for(auto& C : soft_clauses){
        for(Vertex* lit : C.literals){
            pruned.push_back(lit);
        }
    }

    pruned.sort([](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});
    return pruned;
}