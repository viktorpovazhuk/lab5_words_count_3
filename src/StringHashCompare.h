//
// Created by vityha on 26.04.22.
//

#ifndef COUNTER_STRINGHASHCOMPARE_H
#define COUNTER_STRINGHASHCOMPARE_H

#include <cstdio>
#include <string>

struct StringHashCompare {
    static size_t hash( const std::string& x ) {
        size_t h = 0;
        for( const char* s = x.c_str(); *s; ++s )
            h = (h*17)^*s;
        return h;
    }
    //! True if strings are equal
    static bool equal( const std::string& x, const std::string& y ) {
        return x==y;
    }
};

#endif //COUNTER_STRINGHASHCOMPARE_H
