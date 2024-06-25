#include <iostream> 
#include <algorithm>
#include "load.h"

void checkLabelValidity(const string& label) {
    const auto is_valid = [](char c) -> bool {
        return isalnum(static_cast<unsigned char>(c)) || c == '_';
    };

    if (!all_of(label.begin(), label.end(), is_valid)) {
        throw runtime_error("The name of a node includes a disallowed character: " + label);
    }
}

void load_graph(Graph& G){
    //ios::sync_with_stdio(false);
    string line;
    unordered_map<string, Vertex*> label_map;
    vector<string> name_table;
    G.deg_lists.resize(20); //also arbitrary micro-optimization
    while (getline(cin, line)) {
        // Remove everything after the first #-symbol.
        line = line.substr(0, line.find('#'));

        /**
         * Find the start index of the first vertex, the end index of the second vertex, and the index of the whitespace
         * separating the first and second vertex.
         */
        constexpr char whitespace[] = " \n\t\r\v\f";
        size_t start_idx = line.find_first_not_of(whitespace);
        size_t mid_idx = line.find(' ', start_idx);
        size_t end_idx = line.find_last_not_of(whitespace);

        // Continue if the line is empty.
        if (start_idx == string::npos) {
            continue;
        }

        // Throw an error if the indices are messed up.
        if (start_idx >= mid_idx || mid_idx >= end_idx) {
            throw runtime_error("The input data seems to be invalid.");
        }

        // Copy the labels to separate strings.
        std::string u_label = line.substr(start_idx, mid_idx - start_idx);
        std::string v_label = line.substr(mid_idx + 1, end_idx - mid_idx);

        // Check whether the labels have a valid format (throws error if not).
        checkLabelValidity(u_label);
        checkLabelValidity(v_label);

        // Convert from string to vertex.

        Vertex* u = nullptr;
        Vertex* v = nullptr;

        if(label_map.find(u_label) == label_map.end()){
            u = G.add_vertex();
            label_map[u_label] = u;
            name_table.push_back(u_label);
            #if !NDEBUG || DEBUG 
                u->name = move(u_label);
            #endif
        }
        else{
            u = label_map[u_label];
        };

        if(label_map.find(v_label) == label_map.end()){
            v = G.add_vertex();
            label_map[v_label] = v;
            name_table.push_back(v_label);
            #if !NDEBUG || DEBUG
                v->name = move(v_label);
            #endif
        }
        else{
            v = label_map[v_label];
        };

        assert(u != v);

        G.add_edge(u,v);
    }

    G.name_table = name_table;
    G.N = G.total_N;
    //G.M = G.total_M;

}

Graph complementary_graph(Graph& G){
    Graph H;
    H.name_table = G.name_table;

    vector<Vertex*> G_to_H(G.N); // from v in G to v in H

    for(Vertex* v : G.V){
        Vertex* vh = H.add_vertex();
        vh->id = v->id;
        G_to_H[v->id] = vh;
    }

    for(Vertex* v : G.V){
        G.new_timestamp();
        auto v_iter = G.timestamp;
        v->marked = v_iter;         // prevent self loops
        for(Vertex* n : v->neighbors){
            n->marked = G.timestamp;
        }

        for(Vertex* w : G.V){
            if(w->marked != v_iter && w < v){
                H.add_edge(G_to_H[v->id], G_to_H[w->id]);
            }
        }
    }

    H.N = H.total_N;
    G.delete_all();
    return H;
}