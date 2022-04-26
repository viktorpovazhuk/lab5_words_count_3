//
// Created by petro on 27.03.2022.
//

#include "thread_functions.h"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using mapStrInt = std::map<std::string, int>;

//#define SERIAL

/*
overworkFile()

Main func in thread. Take element from the queue, index it and merge with global dict.

indexFile()

Split in words, format and count. Do it in reference of dict.

mergeDicts()

Merge to global dict.*/

void overworkFile(ThreadSafeQueue<std::string> &filesContents, std::unordered_map<std::string, int> &dict,
                  std::mutex &globalDictMutex,
                  std::chrono::time_point<std::chrono::high_resolution_clock> &timeFindingFinish) {

    std::map<std::string, int> localDict;
#ifdef SERIAL
    int fileNumber = 0;
#endif

    while (true) {
        std::string file;
        try {
            file = filesContents.deque();
            if (std::equal(file.begin(), file.end(), "")) {
                // don't need mutex because queue is empty => other threads wait
                timeFindingFinish = get_current_time_fenced();
                filesContents.enque("");
                break;
            }
        } catch (std::error_code e) {
            std::cerr << "Error code " << e << ". Occurred while working with queue in thread." << std::endl;
            continue;
        }
        std::vector<std::string> words;

        indexFile(words, file);

        for (auto &word: words) {
            if (localDict.find(word) != localDict.end()) {
                localDict.find(word)->second += 1;
            } else {
                localDict.insert({word, 1});
            }
        }

        globalDictMutex.lock();
        mergeDicts(dict, localDict);
        globalDictMutex.unlock();
        localDict.clear();

#ifdef SERIAL
        fileNumber++;
        std::cout << fileNumber << "\n";
#endif
    }
}

void indexFile(std::vector<std::string> &words, std::string &file) {

    try {
        std::for_each(file.begin(), file.end(), [](char &c) {
            c = std::tolower(c);
        });
    } catch (std::error_code e) {
        std::cerr << "Error code " << e << ". Occurred while transforming word to lowercase" << std::endl;
    }
    size_t start_pos = 0;
    try {
        while ((start_pos = file.find(std::string("\n"), start_pos)) != std::string::npos) {
            file.replace(start_pos, std::string("\n").length(), std::string(" "));
            start_pos += std::string(" ").length();
        }

        start_pos = 0;
        while ((start_pos = file.find(std::string("\r"), start_pos)) != std::string::npos) {
            file.replace(start_pos, std::string("\r").length(), std::string(" "));
            start_pos += std::string(" ").length();
        }
    } catch (std::error_code e) {
        std::cerr << "Error code " << e << ". Occurred while deleting /n and /r from files" << std::endl;
    }


    try {
        std::stringstream s(file);
        std::string s2;

        while (std::getline(s, s2, ' ')) {
            words.push_back(s2);
        }
    } catch (std::error_code e) {
        std::cerr << "Error code " << e << ". Occurred while splitting file into words" << std::endl;
    }

}

void mergeDicts(ThreadSafeQueue<mapStrInt> &dictsQueue, TimePoint &timeMergingFinish) {
    while (true) {
        mapStrInt dict1 = dictsQueue.deque();
        while (dict1.empty()) {
            if (dictsQueue.empty()) {
                dictsQueue.enque(dict1);
                timeMergingFinish = get_current_time_fenced();
                return;
            } else {
                dictsQueue.enque(dict1);
                dict1 = dictsQueue.deque();
            }
        }
        mapStrInt dict2 = dictsQueue.deque();
        while (dict2.empty()) {
            if (dictsQueue.empty()) {
                dictsQueue.enque(dict1);
                dictsQueue.enque(dict2);
                timeMergingFinish = get_current_time_fenced();
                return;
            } else {
                dictsQueue.enque(dict2);
                dict2 = dictsQueue.deque();
            }
        }

        try {
            for (auto &i: dict1) {
                if (dict2.find(i.first) != dict2.end()) {
                    dict2.at(i.first) += i.second;
                } else {
                    dict2.insert({i.first, i.second});
                }
            }
        } catch (std::error_code &e) {
            std::cerr << "Error code " << e << ". Occurred while merging dicts" << std::endl;
        }

        dictsQueue.enque(std::move(dict2));
    }
}