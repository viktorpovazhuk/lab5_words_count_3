//
// Created by vityha on 27.03.22.
//

#ifndef INDEX_FILES_FILES_METHODS_H
#define INDEX_FILES_FILES_METHODS_H

#include "thread_safe_queue.h"
#include "time_measurement.h"
#include "ReadFile.h"

#include <filesystem>
#include <string>
#include <fstream>
#include <tbb/concurrent_queue.h>

namespace fs = std::filesystem;

void findFiles(std::string &filesDirectory, tbb::concurrent_bounded_queue<fs::path> &paths);
void readFiles(tbb::concurrent_bounded_queue<fs::path> &paths, tbb::concurrent_bounded_queue<ReadFile> &filesContents, std::uintmax_t maxFileSize,  std::chrono::time_point<std::chrono::high_resolution_clock> &timeReadingFinish);

#endif //INDEX_FILES_FILES_METHODS_H
