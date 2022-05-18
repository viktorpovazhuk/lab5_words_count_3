//
// Created by petro on 27.03.2022.
//

#include "thread_functions.h"
#include "StringHashCompare.h"

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_hash_map.h>

namespace fs = std::filesystem;

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using MapStrInt = std::map<std::string, int>;
using BoundedMapQueue = tbb::concurrent_bounded_queue<MapStrInt>;
using BoundedPathQueue = tbb::concurrent_bounded_queue<fs::path>;
using BoundedRFQueue = tbb::concurrent_bounded_queue<ReadFile>;
using StringTable = tbb::concurrent_hash_map<std::string, int, StringHashCompare>;

//#define SERIAL

/*
overworkFile()

Main func in thread. Take element from the queue, index it and merge with global dict.

indexFile()

Split in words, format and count. Do it in reference of dict.

mergeDicts()

Merge to global dict.*/

void overworkFile(tbb::concurrent_bounded_queue<ReadFile> &filesContents, int &numOfWorkingIndexers, std::mutex& numOfWorkingIndexersMutex, tbb::concurrent_bounded_queue<std::map<std::basic_string<char>, int>> &dict,
                  std::chrono::time_point<std::chrono::high_resolution_clock> &timeFindingFinish) {

    std::map<std::basic_string<char>, int> localDict;
#ifdef SERIAL
    int fileNumber = 0;
#endif

    while (true) {
        ReadFile file;
        try {
            filesContents.pop(file);
            if (file.content == "") {
                // don't need mutex because queue is empty => other threads wait
                ReadFile blank;
                timeFindingFinish = get_current_time_fenced();
                filesContents.push(blank);
                break;
            }
        } catch (std::error_code e) {
            std::cerr << "Error code " << e << ". Occurred while working with queue in thread." << std::endl;
            continue;
        }

        if (file.extension == ".zip") {
            struct archive *a;
            struct archive_entry *entry;
            int r;
            int64_t length;
            void *buf;

            a = archive_read_new();
            archive_read_support_filter_all(a);
            archive_read_support_format_all(a);
            std::ifstream raw_file(file.content, std::ios::binary);
            raw_file.seekg(0, std::ios::end);
            size_t file_size = raw_file.tellg();

            r = archive_read_open_filename(a, file.content.c_str(), file_size);
            if (r != ARCHIVE_OK) {
                exit(26);
            }
            while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
                ReadFile readFile;
                std::string name = archive_entry_pathname(entry), ext;
                int p = name.find('.');
                ext = name.substr(p, name.size());
                name = name.substr(0, p);

                if (ext == ".txt") {
                    length = archive_entry_size(entry);
                    buf = malloc(length);
                    archive_read_data(a, buf, length);

                    std::string content{(char *) buf};
                    readFile.content = std::move(content);
                    free(buf);
                    readFile.extension = ".txt";
                    readFile.filename = name;
                    filesContents.push(std::move(readFile));
                }
            }
        } else {
            try {
                file.content = boost::locale::normalize(file.content);
                file.content = boost::locale::fold_case(file.content);

            } catch (std::error_code e) {
                printf("Indexing error");
            }

            boost::locale::boundary::ssegment_index words(boost::locale::boundary::word, file.content.begin(),
                                                          file.content.end());
            words.rule(boost::locale::boundary::word_letters);

            for (auto &word: words) {
                if (localDict.find(word) != localDict.end()) {
                    localDict.find(word)->second += 1;
                } else {
                    localDict.insert({word, 1});
                }
            }

            dict.push(localDict);
            localDict.clear();

#ifdef SERIAL
            fileNumber++;
            std::cout << fileNumber << "\n";
#endif
        }
    }
    if (numOfWorkingIndexers == 1) {
        numOfWorkingIndexersMutex.lock();
        numOfWorkingIndexers = 0;
        numOfWorkingIndexersMutex.unlock();

        localDict.clear();
        dict.push(localDict);
    } else {
        numOfWorkingIndexersMutex.lock();
        numOfWorkingIndexers--;
        numOfWorkingIndexersMutex.unlock();

    }
}


void mergeDicts(StringTable &globalDict, BoundedMapQueue &dicts, TimePoint &timeMergingFinish) {
    MapStrInt localDict;
    dicts.pop(localDict);
    while (!localDict.empty()) {
        try {
            StringTable::accessor a;
            for (auto &i: localDict) {
                globalDict.insert(a, i.first);
                a->second += i.second;
                a.release();
            }
        } catch (std::error_code &e) {
            std::cerr << "Error code " << e << ". Occurred while merging dicts" << std::endl;
        }
        dicts.pop(localDict);
    }
    MapStrInt emptyDict;
    dicts.push(std::move(emptyDict));

    timeMergingFinish = get_current_time_fenced();
}
