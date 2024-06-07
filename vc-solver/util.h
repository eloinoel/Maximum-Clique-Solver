#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <optional>

using namespace std;

using ts = chrono::high_resolution_clock::time_point;

class Counter{
public:
    int count = 0;
    string name;

    Counter() : count(0) {}

    void start(string msg = "", int val = 0){
        count = val; 
        name = msg;
    }

    void report(){
        cout << "[COUNTER] " << name << ": " << count << "\n";
    }

    Counter& operator++(int){
        count++;
        return *this;
    }

    Counter& operator--(int){
        count--;
        return *this;
    }
};

class Timer{
public:

    Timer(){}

    map<string, pair<ts, optional<ts>>> time_table;

    void start(string name, bool overwrite = false){        
        ts time = chrono::high_resolution_clock::now();

        if(time_table.find(name) != time_table.end() && !overwrite)
            assert(false && "repeat start of timer without overwrite");

        time_table[name] = {time, {}};

    }

    void end(string name, bool overwrite = false){
        ts time = chrono::high_resolution_clock::now();
        auto entry = time_table.find(name);
        if(entry == time_table.end())
            assert(false && "ending unstarted timer");

        if((entry->second).second && !overwrite){
            assert(false && "repeat start of timer without overwrite");
        }

        entry->second.second = time;
    }
    
    template<typename T = chrono::microseconds>
    void report(string name, bool end_by_default = false){
        ts time = chrono::high_resolution_clock::now();
        auto entry = time_table.find(name);
        if(entry == time_table.end() && !end_by_default)
            assert(false && "reporting non-existing timer");
        
        if(!(entry->second).second){
            if(end_by_default){
                entry->second.second = time;
            }
            else{
                assert(false && "reporting unended timer without end_by_default flag");
            }
        }

        auto t = entry->second;
        auto count = chrono::duration_cast<T>(t.second.value() - t.first).count();
        cout << "[TIMER: " << name << "] " << count << get_time_string<T>() << "\n" << flush; 
    }

    template <typename T>
    string get_time_string(){
        if constexpr (is_same<T, std::chrono::microseconds>::value){
            return "\u03BCs";
        } else if constexpr (is_same<T, std::chrono::nanoseconds>::value){
            return "ns";
        } else if constexpr (is_same<T, std::chrono::milliseconds>::value){
            return "ms";
        } else if constexpr (is_same<T, std::chrono::seconds>::value){
            return "s";
        } else {
            return "unknown timeformat";
        }
    }
};