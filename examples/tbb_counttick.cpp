//
// Created by vityha on 26.04.22.
//

#include <oneapi/tbb/tick_count.h>
#include <iostream>

#include "oneapi/tbb/concurrent_hash_map.h"
#include "oneapi/tbb/blocked_range.h"
#include "oneapi/tbb/parallel_for.h"
#include <oneapi/tbb/concurrent_queue.h>
#include <string>


using namespace oneapi::tbb;
using namespace std;


// Structure that defines hashing and comparison operations for user's type.
struct MyHashCompare {
    static size_t hash(const string &x) {
        size_t h = 0;
        for (const char *s = x.c_str(); *s; ++s)
            h = (h * 17) ^ *s;
        return h;
    }

    //! True if strings are equal
    static bool equal(const string &x, const string &y) {
        return x == y;
    }
};


// A concurrent hash table that maps strings to ints.
typedef concurrent_hash_map<string, int> StringTable; // ,MyHashCompare


// Function object for counting occurrences of strings.
struct Tally {
    StringTable &table;

    Tally(StringTable &table_) : table(table_) {}

    void operator()(const blocked_range<string *> range) const {
        for (string *p = range.begin(); p != range.end(); ++p) {
            StringTable::accessor a;
            table.insert(a, *p);
            a->second += 1;
        }
    }
};


const size_t N = 100;


string Data[N];


void CountOccurrences() {
    // Construct empty table.
    StringTable table;

    for (auto &i: Data) {
        i = "a";
    }


    // Put occurrences into the table
    parallel_for(blocked_range<string *>(Data, Data + N),
                 Tally(table));


    // Display the occurrences
    for (StringTable::iterator i = table.begin(); i != table.end(); ++i)
        printf("%s %d\n", i->first.c_str(), i->second);
}

int main() {
//    StringTable table;
//    string str = "abc";
//
//    StringTable::accessor a;
//    table.insert( a, str );
//    a->second += 1;
//
//    for( StringTable::iterator i=table.begin(); i!=table.end(); ++i )
//        printf("%s %d\n",i->first.c_str(),i->second);

//    CountOccurrences();

    oneapi::tbb::concurrent_bounded_queue<string> Q;
    Q.set_capacity(2);
    for (int i = 0; i < 2; i++) {
        Q.push("a");
    }
    string s;
    std::cout << Q.size() << "\n";
    int n = Q.size();
    for (int i = 0; i < n; i++) {
        Q.pop(s);
        std::cout << s << "!\n";
    }

    return 0;
}