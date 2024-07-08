#include "graph.h"
#include <vector>
#include <iostream>
#include "degeneracy_ordering.h"
#include <future>
#include <thread>
#include "cli_solve.h"
#include "solve_via_vc.h"
#include "lmc_solver.h"
#include "utility"
#include "classifier.h"

enum KERNEL{
    RBF,
    LINEAR,
    POLY
};

inline int next2(int befor, Graph& G){
    for(int i = max(0, befor-1); i<G.deg_lists.size(); i++){
        if(!G.deg_lists[i].empty())return i;
    }
    return -1;
}

inline std::pair<int, int> get_lb_and_degeneracy(Graph &G)
{
    G.set_restore();
    int lb = 0;
    unordered_map<Vertex*, int> core_number;
    int N = 0;
    for(auto l: G.deg_lists)N+=l.size();

    if(N==0)return pair(0,0);
    G.set_restore();
    int cur_core = next2(0,G);

    vector<Vertex*> ordering;
    vector<Vertex*> C_0;
    ordering.reserve(N);
    int i = cur_core;
    int k = 0;
    for(; i!=-1;i=next2(i,G)){
        k++;
        Vertex* v = G.deg_lists[i].back();
        
        if(i > cur_core){
            cur_core=i;
        }
        core_number[v]=cur_core;
        if(i == N-k){
            for(auto l: G.deg_lists){
                ordering.insert(ordering.end(), l.begin(), l.end());
                C_0.insert(C_0.end(), l.begin(), l.end());
                for(auto e:l){
                    core_number[e]=cur_core;
                }
            }
            break;
        }
        ordering.push_back(v);
        G.MM_discard_vertex(v);
    }
    G.restore();

    if(C_0.size()>lb){
        lb=C_0.size();
    }
    vector<Vertex*> new_ordering;
    int max_core = -1;
    for(Vertex* v : ordering){
        max_core = max(max_core, core_number[v]);
        if(core_number[v]<lb){
            G.MM_discard_vertex(v);
        }else {
            new_ordering.push_back(v);
        }
    }
    G.restore();
    return pair(C_0.size(),max_core);
}

inline Predict_status sgn(float arg){

    if(arg>=0)return TIMEOUT;
    return NO_TIMEOUT;
}

float rbf_kernel(vector<float> input, vector<float> support_vectors, float gamma){
    float sum = 0;
    for(int i =0; i< input.size(); i++){
        sum+= (input[i] - support_vectors[i])*(input[i] - support_vectors[i]);
    }
    return exp(-gamma * sum);
}

float linear_kernel(vector<float> input, vector<float> support_vectors){
    float sum = 0;
    for(int i =0; i< input.size(); i++){
        sum+= input[i]*support_vectors[i];
    }
    return sum;
}


float poly_kernel(vector<float> input, vector<float> support_vectors, float gamma, float coef, int degree) {
    float sum = 0;
    for (int i = 0; i < input.size(); i++) {
        sum += input[i] * support_vectors[i];
    }
    return pow(gamma * sum + coef, degree);
}

inline float intern_predict(vector<vector<float>> X, vector<float> input, vector<int> support_vector_indices, vector<float> dual_Coeff, float intercept, KERNEL kernel, float gamma = 0, float coef = 0, int degree = 0){
    float sum = intercept;
    if(kernel == RBF){
        for(int i =0; i<support_vector_indices.size(); i++){
            sum+= dual_Coeff[i] * rbf_kernel(input, X[support_vector_indices[i]], gamma);
        }
    }
    if(kernel == LINEAR){
        for(int i =0; i<support_vector_indices.size(); i++){
            sum+= dual_Coeff[i] * linear_kernel(input, X[support_vector_indices[i]]);
        } 
    }
    if(kernel == POLY){
        for(int i =0; i<support_vector_indices.size(); i++){
            sum+= dual_Coeff[i] * poly_kernel(input, X[support_vector_indices[i]], gamma, coef, degree);
        }   
    }
    return sum;
}

Expected_times Classifier::predict_timeout(Graph &G)
{

    std::vector<float> input;
    pair<int, int> lb_maxcore = get_lb_and_degeneracy(G);

    input.push_back((float) G.N);
    input.push_back((float) G.M);
    input.push_back((float) G.max_degree);
    input.push_back((float) G.min_degree);
    input.push_back((float) lb_maxcore.second);
    input.push_back((float) lb_maxcore.first);


    Predict_status LMC_prediction = sgn(intern_predict(X,input,LMC_support_vector,LMC_DC,LMC_intercept, POLY, LMC_gamma, LMC_coef, LMC_degree));
    Predict_status dOmega_prediction = sgn(intern_predict(X,input,dOmega_support_vector,dOmega_DC,dOmega_intercept, RBF, dOmega_gamma));
    Predict_status clisat_prediction = sgn(intern_predict(X,input,clisat_support_vector,clisat_DC,clisat_intercept,RBF,clisat_gamma));
    Predict_status vc_comp_prediction = sgn(intern_predict(X,input,vc_comp_support_vector,vc_comp_DC,vc_comp_intercept, POLY, vc_comp_gamma, vc_comp_coef, vc_comp_degree));
   
    
    return Expected_times( LMC_prediction, dOmega_prediction, clisat_prediction, vc_comp_prediction);
}

