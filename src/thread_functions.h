//
// Created by petro on 27.03.2022.
//

#ifndef SERIAL_THREAD_FUNCTIONS_H
#define SERIAL_THREAD_FUNCTIONS_H

#include "time_measurement.h"
#include "StringHashCompare.h"
#include "thread_safe_queue.h"

#include <oneapi/tbb/concurrent_queue.h>
#include <oneapi/tbb/concurrent_hash_map.h>

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <locale>



void overworkFile(ThreadSafeQueue<std::string> &filesContents, std::unordered_map<std::string, int>& dict, std::mutex &mut, std::chrono::time_point<std::chrono::high_resolution_clock> &timeFindingFinish);

void indexFile(std::vector <std::string> &words, std::string& file);

void mergeDicts(oneapi::tbb::concurrent_hash_map<std::string, int, StringHashCompare> &globalDict, oneapi::tbb::concurrent_bounded_queue<std::map<std::string, int>> &dicts, std::chrono::time_point<std::chrono::high_resolution_clock> &timeMergingFinish);

#endif //SERIAL_THREAD_FUNCTIONS_H
