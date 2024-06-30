#include <algorithm>
#include "inc_max_sat.h"
#include "graph.h";

inline vector<vector<int>> conflict_detection(vector<vector<int>>& F, vector<vector<int>>& S){
    std::vector<int> removed;
    std::unordered_map<int, std::vector<int>> removed_of;
    std::unordered_map<int, std::vector<int>> reason;
    reason.reserve(S.size());   
    std::vector<std::vector<int>> Q;
    std::vector<std::vector<int>> I;

    while(!S.empty()){
        vector<int> u = S.back();
        S.pop_back();
        int l = u.back();
        reason[l]=u;
        reason[-l]=u;

        //remove satisfied clausels
        for(vector<int> c: F){
            auto it = find(c.begin(), c.end(), l);
            if(it != c.end()){
                F.erase(find(F.begin(), F.end(), c));
            }
        }

        //for clausel c in phi containing -l
        for(vector<int> c: F){
            auto it = find(c.begin(), c.end(), -l);
            if(it!=c.end()){

            //remove -l from c
            c.erase(it);
            removed.push_back(-l);
            removed_of[-l] = c;


            if(c.size()==1)S.push_back(c);
            else if(c.size()==0){
                Q.clear();
                I.clear();
                Q.insert(Q.begin(), c);
                I.push_back(c);
                while(!Q.empty()){
                    vector<int> c_prime = Q.back();
                    Q.pop_back();
                    
                    //for removed l' of c'
                    for(int l_prime: removed){
                        if(removed_of[l_prime]==c_prime){

                            //add reason r for l' to Q if not in yet
                            vector<int> r = reason[l_prime];
                            auto it2 = find(I.begin(),I.end(), r);
                                if(it2!=I.end()){
                                Q.insert(Q.begin(),r);
                                I.push_back(r);
                            }
                        }

                    }

                }
                return I;
            }
            }
        }
    }
    return vector<vector<int>>();
}

vector<Vertex*> inc_max_sat(vector<Vertex*>& ordering, std::unordered_map<Vertex*, int> &coloring, int r){
    int z_index = ordering.size();

    //adding soft clausels and B
    vector<vector<int>> soft_clausels(r);

    for(int i =0; i<ordering.size(); i++){
        int color = min(coloring[ordering[i]], r);
        soft_clausels[color-1].push_back(i);
    }

    //remove B from soft clausels
    vector<int> B = soft_clausels.back(); //coded as index bc its easier in the following
    soft_clausels.pop_back();
    vector<Vertex*> real_B;
    for(int i: B){
        real_B.push_back(ordering[i]);
    }


    //adding hard clausels      
    vector<vector<int>> phi;
    phi.reserve(z_index*z_index);
    for(int i =0; i<ordering.size(); i++){
        for(int j =i+1; j<ordering.size(); j++){
            if(ordering[i]->adjacent_to(ordering[j])){
                phi.push_back({-i, -j});
            }
        }
    }

    //adding soft clausels to phi
    phi.insert(phi.end(),soft_clausels.begin(), soft_clausels.end());

    while(!B.empty()){
        int v = B.back();

        //add clausel {v} to phi
        vector<int> unit_clausel={v};
        phi.push_back(unit_clausel);
        soft_clausels.push_back(unit_clausel);

        //create stack S with all unit clausels from phi
        vector<vector<int>> S;
        for(vector<int> clausel: phi){
            if(clausel.size()==1){
                S.push_back(clausel);
            }
        }

        vector<vector<int>> conflict_clausels = conflict_detection(phi, S);

        //filter soft clausels from conflict clausels
        vector<vector<int>> soft_conflict_clausels;
        for(vector<int> clausel: soft_clausels){
            auto it= find(conflict_clausels.begin(), conflict_clausels.end(), clausel);
            if(it!=conflict_clausels.end())soft_conflict_clausels.push_back(clausel);
        }

        if(!soft_conflict_clausels.empty()){
            
            //delete c_1 ... c_k from phi
            for(vector<int> sc: soft_conflict_clausels){
                phi.erase(find(phi.begin(), phi.end(), sc));
                soft_clausels.erase(find(soft_clausels.begin(), soft_clausels.end(), sc));
            }

            //add c_1 or z_1, ..., c_k or z_k to phi
            for(int i =0; i<soft_conflict_clausels.size(); i++){
                soft_conflict_clausels[i].push_back(z_index + i);
                soft_clausels.push_back(soft_conflict_clausels[i]);
                phi.push_back(soft_conflict_clausels[i]);
            }

            //add z_1 or ... or z_k to hard clausels
            vector<int> Z;
            for(int i=0; i<soft_conflict_clausels.size(); i++){
                Z.push_back(z_index + i);

            }
            phi.push_back(Z);

            //add not z_i or not z_j to hard clausels
            for(int i =0; i<soft_conflict_clausels.size(); i++){
                for(int j = i+1; j<soft_conflict_clausels.size(); j++){
                    phi.push_back({-(i + z_index),-(j + z_index)});
                }
            }
            z_index+=soft_conflict_clausels.size();
            
            
            B.pop_back();
        }else break;
    }


    return real_B;
}

