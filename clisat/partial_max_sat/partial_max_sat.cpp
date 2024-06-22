#include "partial_max_sat.h"
#include <algorithm> // temp for sort...
#include <set> //temp...
#define PSAT_USED 33123 // arbitrary constant, used to see if something (wrongly) keeps it later

#define SELF_EMPTY -2
#define NONE_EMPTY -1

list<Vertex*> Partial_Max_Sat::SATCOL(Graph& G){
    // TODO: remove need to sort here
    sort(branchset.begin(), branchset.end(), [](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});
    
    for(Vertex* b : branchset)
        b->data.iset.iclass = -1; 

    vector<Vertex*>& remaining_B = branchset; // intentional copy? i dnnt remember why I wanted to copy

    while(!remaining_B.empty()){
        /* ISEQ ITERATION START */
        auto in_b = G.new_timestamp();

        for(Vertex* b : remaining_B)
            b->marked = in_b;

        remaining_B[0]->data.iset.iclass = PSAT_USED;

        for(size_t i = 1; i < remaining_B.size(); i++){
            Vertex* b = remaining_B[i];
            for(Vertex* n : b->neighbors){
                if(n->marked == in_b && n->data.iset.iclass == PSAT_USED)
                    goto fail;
            }

            b->data.iset.iclass = PSAT_USED;

            fail:
            continue;
        }
        /* ISEQ ITERATION END */

        vector<Vertex*> new_iset;
        vector<Vertex*> rest;
        // split B
        for(Vertex* b : remaining_B){
            if(b->data.iset.iclass == PSAT_USED){
                new_iset.push_back(b);
            }
            else{
                rest.push_back(b);
            }
        }

        if(!detect_conflict_iset(G, new_iset)){
            cout << "did not find conflict\n";
            return output_pruned_to_list();
        }

        remaining_B = move(rest);
    }

    return output_pruned_to_list();
}

// b_iset = the independent set to check for failed literals
bool Partial_Max_Sat::detect_conflict_iset(Graph& G,vector<Vertex*>& b_iset){

    auto old_clauses = soft_clauses; // copy for later, not worth it to undo all the changes
    vector<int> conflict;
    set<int> conflicting_subsets;

    for(Vertex* b : b_iset){
        auto adj_b = G.new_timestamp();
        for(Vertex* n : b->neighbors){
            n->marked = adj_b;
        }

        vector<int> units;
        for(size_t i = 0; i < soft_clauses.size(); i++){
            auto& clause = soft_clauses[i].literals;

            size_t j = 0;
            while(j < clause.size()){
                Vertex* literal = clause[j];
                if(literal->marked != adj_b){
                    swap(clause[j], clause.back());
                    clause.pop_back();
                }else{
                    j++;
                }
            }

            int try_use;
            if(clause.empty() && (try_use =use_slack(soft_clauses[i])) != NONE_EMPTY){
                if(try_use != SELF_EMPTY)
                    conflicting_subsets.insert(try_use);
                conflicting_subsets.insert(i); // TODO: did I just forge this before or is this incorrect?
                goto success;
            }

            if((clause.size() + soft_clauses[i].slack_remaining) == 1)
                units.push_back(i);
        }

        /* try unit propagation with new clauses */
        if(units.empty()){
           soft_clauses = old_clauses;
           return false;
        }

        conflict = propagate_units(G, units);
        if(!conflict.empty()){
            for(int c : conflict)
                conflicting_subsets.insert(c);
        }
        else{
            return false;
        }
        success:
        soft_clauses = old_clauses;
        //TODO: keep track of clauses involved in conflict
        continue;

    }

    /* the independent set b_iset causes a conflict
         * add b_iset to soft clauses
         * transform graph to solve the conflict by adding new literals
         */
    soft_clauses = old_clauses;

    soft_clauses.push_back({move(b_iset), {}});

    vector<pair<int, int>> constraint; 
    constraint.reserve(conflicting_subsets.size() + 1);

    for(auto& c : conflicting_subsets){
        auto& clause = soft_clauses[c];

        constraint.  push_back( {c, (int) clause.slack.size()} );
        clause.slack.push_back(slack_constraint.size());
        clause.slack_remaining++;
    }

    auto& b_clause = soft_clauses.back();
    
    constraint.  push_back( {soft_clauses.size(), 0} );
    b_clause.slack.push_back(slack_constraint.size());
    b_clause.slack_remaining++;

    slack_constraint.push_back(move(constraint));

    return true;
}

vector<int> Partial_Max_Sat::propagate_units(Graph& G, vector<int>& units){
    vector<int> conflict_subset; //FIXME: unsure if this works, the paper makes it seem more complicated
    for(size_t i = 0; i < units.size(); i++){
        assert((soft_clauses[units[i]].literals.size() + soft_clauses[i].slack_remaining == 1));
        //if(!soft_clauses[units[i]].active)
        //    continue; not needed?
        soft_clauses[units[i]].active = false; // this clause is fulfilled now
        if(soft_clauses[units[i]].literals.size() == 0){
            int try_use = use_slack(soft_clauses[i]);
            conflict_subset.push_back(i);
            if(try_use != NONE_EMPTY){
                if(try_use != SELF_EMPTY)
                    conflict_subset.push_back(try_use);
                return conflict_subset;
            }
            continue;
        }
        Vertex* unit = soft_clauses[units[i]].literals[0];

        conflict_subset.push_back(i);

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
            bool was_unit = (clause.size() + soft_clauses[k].slack_remaining) == 1;
            while(j < clause.size()){
                Vertex* literal = clause[j];
                if(literal->marked != adj_unit){
                    swap(clause[j], clause.back());
                    clause.pop_back();
                }else{
                    j++;
                }
            }

            int try_use;
            if(clause.empty() && (try_use = use_slack(soft_clauses[k])) != NONE_EMPTY){
                if(try_use != SELF_EMPTY)
                    conflict_subset.push_back(try_use);
                conflict_subset.push_back(k);
                return conflict_subset;
            }

            if((clause.size() + soft_clauses[k].slack_remaining) == 1 && !was_unit)
                units.push_back(k);
        }
        
    }

    return {};
}


// returns true if slack could be used, and that slack usage does not create a different empty clause
int Partial_Max_Sat::use_slack(Clause& C){
    if(C.slack_remaining == 0)
        return SELF_EMPTY; // no slack to use
    else{
        auto& slacks = C.slack;
        size_t i = 0;
        for(; i < slacks.size(); i++){
            if(slacks[i] != -1)
                break;
            i++;
        }
        int active_slack = slacks[i];

        for(auto& [clause_idx, slack_idx] : slack_constraint[active_slack]){
            Clause& CS = soft_clauses[clause_idx];
            CS.slack_remaining--;
            if(CS.slack_remaining == 0 && CS.literals.size() == 0)
                return clause_idx; // CS is empty
            
            CS.slack[slack_idx] = -1;
        }
    }
    
    return NONE_EMPTY; // able to use slack without causing an empty clause
}


list<Vertex*> Partial_Max_Sat::output_pruned_to_list(){
    list<Vertex*> pruned;
    for(auto& C : soft_clauses){
        for(Vertex* lit : C.literals){
            pruned.push_back(lit);
        }
    }

    pruned.sort([](Vertex* a, Vertex* b){return a->order_pos < b->order_pos;});
    return pruned;
}