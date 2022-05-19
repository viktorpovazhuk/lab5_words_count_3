//
// Created by vityha on 27.03.22.
//

#include "files_methods.h"
#include "ReadFile.h"

#include <tbb/concurrent_queue.h>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using BoundedPathQueue = tbb::concurrent_bounded_queue<fs::path>;
using BoundedRFQueue = tbb::concurrent_bounded_queue<ReadFile>;

void findFiles(std::string &filesDirectory, BoundedPathQueue &paths) {
    for (const auto &dir_entry: fs::recursive_directory_iterator(filesDirectory)) {
        if (dir_entry.path().extension() == ".zip" || dir_entry.path().extension() == ".txt") {
            paths.push(dir_entry.path());
        }
    }
    paths.push(fs::path(""));
}

void readFiles(BoundedPathQueue &paths, BoundedRFQueue &filesContents, std::uintmax_t maxFileSize, TimePoint &timeReadingFinish) {
    fs::path path;
    paths.pop(path);
    while (path != fs::path("")) {
        if (fs::file_size(path) >= maxFileSize){
            paths.pop(path);
            continue;
        }
        ReadFile readFile;
        if (path.extension() == ".txt") {
            std::ifstream ifs(path);
            std::string content{(std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>())};
            readFile.content = std::move(content);
            readFile.extension = path.extension();
            readFile.filename = path.filename();
            filesContents.push(std::move(readFile));
        }
        else if (path.extension() == ".zip") {
            readFile.content = path;
            readFile.extension = path.extension();
            readFile.filename = path.filename();
            filesContents.push(std::move(readFile));
        }

        paths.pop(path);
    }
    ReadFile emptyReadFile;
    filesContents.push(std::move(emptyReadFile));
    paths.push(fs::path(""));

    timeReadingFinish = get_current_time_fenced();
}