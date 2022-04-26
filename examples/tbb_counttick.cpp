//
// Created by vityha on 26.04.22.
//

#include <oneapi/tbb/tick_count.h>
#include <iostream>

int main(){
    oneapi::tbb::tick_count t0;
    std::cout << t0.resolution();
    return 0;
}