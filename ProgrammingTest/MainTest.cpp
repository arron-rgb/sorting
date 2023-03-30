// Copyright Mass Media. All rights reserved. DO NOT redistribute.

////////////////////////////////////////////////////////////////////////////////////////////////////
// Task List
////////////////////////////////////////////////////////////////////////////////////////////////////
// Notes
//	* This test requires a compiler with C++17 support and was built for Visual Studio 2017.
// 		* Tested on Linux (Ubuntu 20.04) with: g++ -Wall -Wextra -pthread -std=c++17 MainTest.cpp
//		* Tested on macOS Big Sur, 11.0.1 and latest XCode updates.
//	* Correct output for all three sorts is in the CorrectOutput folder. BeyondCompare is recommended for comparing output.
//	* Functions, classes, and algorithms can be added and changed as needed.
//	* DO NOT use std::sort().
// Objectives
//	* 20 points - Make the program produce a SingleAscending.txt file that matches CorrectOutput/SingleAscending.txt. ok
//	* 10 points - Make the program produce a SingleDescending.txt file that matches CorrectOutput/SingleDescending.txt. ok
//	* 10 points - Make the program produce a SingleLastLetter.txt file that matches CorrectOutput/SingleLastLetter.txt. ok
//	* 20 points - Write a brief report on what you found, what you did, and what other changes to the code you'd recommend.
//	* 10 points - Make the program produce three MultiXXX.txt files that match the equivalent files in CorrectOutput; it must be multithreading.
//	* 20 points - Improve performance as much as possible on both single-threaded and multithreading versions; speed is more important than memory usage. ok
//	* 10 points - Improve safety and stability; fix memory leaks and handle unexpected input and edge cases.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <ctime>
#include <utility>
#include <vector>

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#   if defined(__cpp_lib_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#   elif defined(__cpp_lib_experimental_filesystem)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif !defined(__has_include)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   elif __has_include(<filesystem>)
#       ifdef _MSC_VER
#           if __has_include(<yvals_core.h>)
#               include <yvals_core.h>
#               if defined(_HAS_CXX17) && _HAS_CXX17
#                   define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#               endif
#           endif
#           ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#               define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#           endif
#       else
#           define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#       endif
#   elif __has_include(<experimental/filesystem>)
#       define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#   else
#       error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#   endif
#   if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#       include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#   else

# include <filesystem>
# include <sys/stat.h>

#		if __APPLE__
namespace fs = std::__fs::filesystem;
#		else
namespace fs = std::filesystem;
#		endif
#   endif
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions and Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MULTITHREADED_ENABLED 1

enum class ESortType {
    AlphabeticalAscending, AlphabeticalDescending, LastLetterAscending
};
static const string AllSortTypesString = "ESortType::AlphabeticalAscending, ESortType::AlphabeticalDescending, ESortType::LastLetterAscending";


class SortType {
private:
    string sortName;
    ESortType _sortType;

public:
    SortType(string sortName, ESortType type, vector<string> (*SortFunction)(vector<string>, ESortType)) {
        this->sortName = std::move(sortName);
        this->_sortType = type;
        this->sort = SortFunction;
    }

    vector<string> (*sort)(vector<string>, ESortType);

    string getSortName() const {
        return this->sortName;
    }

    ESortType getSortType() {
        return this->_sortType;
    }

};

class IStringComparer {
public:
    virtual bool IsFirstAboveSecond(string _first, string _second) = 0;
};

class AlphabeticalAscendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second) override;
};

class AlphabeticalDescendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second) override;
};

class LastLetterAscendingStringComparer : public IStringComparer {
public:
    bool IsFirstAboveSecond(string _first, string _second) override;
};

static inline std::string &ltrim(std::string &s) {
    if (!s.empty() && s[s.size() - 1] == '\r')
        s.erase(s.size() - 1);
//    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char c) { return c != '\r'; }));
    return s;
}

void DoSingleThreaded(const vector<string> &_fileList, ESortType _sortType, const string &_outputName,
                      vector<string> (*sort)(vector<string>, ESortType));

void DoMultiThreaded(const vector<string> &_fileList, ESortType _sortType, const string &_outputName,
                     vector<string> (*sort)(vector<string>, ESortType));

vector<string> ReadFile(const string &_fileName);

void ThreadedReadFile(const string &_fileName, vector<string> *_listOut);

vector<string>
ThreadedSortFile(ESortType _sortType, vector<string> *_listToSort, vector<string> (*sort)(vector<string>, ESortType));

vector<string> BubbleSort(vector<string> _listToSort, ESortType _sortType);

void QuickSort(vector<string> &_listToSort, int left, int right, IStringComparer *stringComparer);

vector<string> QuickSort(vector<string> _listToSort, ESortType _sortType);

void WriteAndPrintResults(const vector<string> &_masterStringList, const string &_outputName, int _clocksTaken);

vector<string> mergeTwoVector(vector<string> &o1, vector<string> &o2, IStringComparer *stringComparer);

vector<string> mergeVector(vector<vector<string>> &lists, ESortType _sortType);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    // Enumerate the directory for input files
    vector<string> fileList;
    string inputDirectoryPath = "../InputFiles";
    for (const auto &entry: fs::directory_iterator(inputDirectoryPath)) {
        if (!fs::is_directory(entry)) {
            fileList.push_back(entry.path().string());
        }
    }
    vector<SortType> types = {*new SortType("Ascending", ESortType::AlphabeticalAscending, QuickSort),
                              *new SortType("Descending", ESortType::AlphabeticalDescending, QuickSort),
                              *new SortType("LastLetter", ESortType::LastLetterAscending, QuickSort)
    };
    // Do the stuff
    for (auto item: types) {
        DoSingleThreaded(fileList, item.getSortType(), item.getSortName(), item.sort);
    }

#if MULTITHREADED_ENABLED
    cout << endl;
    for (auto item: types) {
        DoMultiThreaded(fileList, item.getSortType(), item.getSortName(), item.sort);
    }
#endif

    // Wait
    cout << endl << "Finished..." << endl;
//	getchar();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Stuff
////////////////////////////////////////////////////////////////////////////////////////////////////
void DoSingleThreaded(const vector<string> &_fileList, ESortType _sortType, const string &_outputName,
                      vector<string> (*sort)(vector<string>, ESortType)) {
    clock_t startTime = clock();
    vector<string> masterStringList;
    for (const auto &i: _fileList) {
        vector<string> fileStringList = ReadFile(i);
        for (const auto &j: fileStringList) {
            masterStringList.push_back(j);
        }
        masterStringList = sort(masterStringList, _sortType);
    }
    clock_t endTime = clock();

    WriteAndPrintResults(masterStringList, "Single" + _outputName, endTime - startTime);
}

void DoMultiThreaded(const vector<string> &_fileList, ESortType _sortType, const string &_outputName,
                     vector<string> (*sort)(vector<string>, ESortType)) {

    clock_t startTime = clock();
    vector<vector<string>> masterStringList(_fileList.size());
    vector<thread> workerThreads(_fileList.size());
    for (unsigned int i = 0; i < _fileList.size(); ++i) {
        workerThreads[i] = thread(ThreadedReadFile, _fileList[i], &masterStringList[i]);
    }
    for (auto &item: workerThreads) {
        try {
            if (item.joinable()) {
                item.join();
            }
        } catch (...) {
            cout << "error" << &item << endl;
        }
    }

    for (unsigned int i = 0; i < _fileList.size(); ++i) {
        workerThreads[i] = thread(ThreadedSortFile, _sortType, &masterStringList[i], sort);
    }
    for (auto &item: workerThreads) {
        try {
            if (item.joinable()) {
                item.join();
            }
        } catch (...) {
            cout << "error" << &item << endl;
        }
    }


    vector<string> outList = mergeVector(masterStringList, _sortType);
    clock_t endTime = clock();
    outList = QuickSort(outList, _sortType);
    WriteAndPrintResults(outList, "Multi" + _outputName, endTime - startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File Processing
////////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> ReadFile(const string &_fileName) {
    vector<string> listOut;
    streampos positionInFile = 0;
    bool endOfFile = false;
    while (!endOfFile) {
        ifstream fileIn(_fileName, ifstream::in);
        fileIn.seekg(positionInFile, ios::beg);

        auto *tempString = new string();
        getline(fileIn, *tempString);

        endOfFile = fileIn.peek() == EOF;
        positionInFile = endOfFile ? ios::beg : fileIn.tellg();

        listOut.push_back(ltrim(*tempString));

        fileIn.close();
    }
    return listOut;
}

void ThreadedReadFile(const string &_fileName, vector<string> *_listOut) {
    *_listOut = ReadFile(_fileName);
}

vector<string>
ThreadedSortFile(ESortType _sortType, vector<string> *_listToSort, vector<string> (*sort)(vector<string>, ESortType)) {
    return sort(*_listToSort, _sortType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sorting
////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlphabeticalAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    unsigned int i = 0;
    while (i < _first.length() && i < _second.length()) {
        if (_first[i] < _second[i])
            return true;
        else if (_first[i] > _second[i])
            return false;
        ++i;
    }
    return i != _second.length();
}

bool AlphabeticalDescendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    unsigned int i = 0;
    while (i < _first.length() && i < _second.length()) {
        if (_first[i] > _second[i])
            return true;
        else if (_first[i] < _second[i])
            return false;
        ++i;
    }
    return (i == _second.length());
}

bool LastLetterAscendingStringComparer::IsFirstAboveSecond(string _first, string _second) {
    unsigned long i = _first.length();
    unsigned long j = _second.length();
    while (true) {
        --i;
        --j;
        if (_first[i] < _second[j])
            return true;
        else if (_first[i] > _second[j])
            return false;
        if (i == 0 || j == 0) {
            break;
        }
    }

    return j - i;
}

vector<string> QuickSort(vector<string> _listToSort, ESortType _sortType) {
    IStringComparer *stringSorter = nullptr;
    if (_sortType == ESortType::AlphabeticalAscending) {
        stringSorter = new AlphabeticalAscendingStringComparer();
    } else if (_sortType == ESortType::AlphabeticalDescending) {
        stringSorter = new AlphabeticalDescendingStringComparer();
    } else if (_sortType == ESortType::LastLetterAscending) {
        stringSorter = new LastLetterAscendingStringComparer();
    }
    if (stringSorter == nullptr) {
        throw runtime_error("Sort Type must be one of the following types" + AllSortTypesString);
    }
    vector<string> &sortedList = _listToSort;
    unsigned long n = _listToSort.size() - 1;
    QuickSort(sortedList, 0, n, stringSorter);
    return sortedList;
}

void QuickSort(vector<string> &_listToSort, int left, int right, IStringComparer *stringComparer) {
    if (left >= right) {
        return;
    }
    std::string pivot = _listToSort[left];
    int i = left;
    int j = right;
    while (i < j) {
        while (i < j && stringComparer->IsFirstAboveSecond(pivot, _listToSort[j])) {
            j--;
        }
        _listToSort[i] = _listToSort[j];
        while (i < j && !stringComparer->IsFirstAboveSecond(pivot, _listToSort[i])) {
            i++;
        }
        _listToSort[j] = _listToSort[i];
    }
    _listToSort[i] = pivot;
    QuickSort(_listToSort, left, i - 1, stringComparer);
    QuickSort(_listToSort, i + 1, right, stringComparer);
}

vector<string> BubbleSort(vector<string> _listToSort, ESortType _sortType) {
    IStringComparer *stringSorter;
    if (_sortType == ESortType::AlphabeticalAscending) {
        stringSorter = new AlphabeticalAscendingStringComparer();
    } else if (_sortType == ESortType::AlphabeticalDescending) {
        stringSorter = new AlphabeticalDescendingStringComparer();
    } else if (_sortType == ESortType::LastLetterAscending) {
        stringSorter = new LastLetterAscendingStringComparer();
    }

    vector<string> sortedList = _listToSort;

    for (unsigned int i = 0; i < sortedList.size() - 1; ++i) {
        for (unsigned int j = 0; j < sortedList.size() - i - 1; ++j) {
            if (!stringSorter->IsFirstAboveSecond(sortedList[j], sortedList[j + 1])) {
                string tempString = sortedList[j];
                sortedList[j] = sortedList[j + 1];
                sortedList[j + 1] = tempString;
            }
        }
    }
    return sortedList;
}

vector<string> mergeVector(vector<vector<string>> &lists, ESortType _sortType) {
    if (lists.empty()) {
        return {};
    }
    IStringComparer *stringSorter;
    if (_sortType == ESortType::AlphabeticalAscending) {
        stringSorter = new AlphabeticalAscendingStringComparer();
    } else if (_sortType == ESortType::AlphabeticalDescending) {
        stringSorter = new AlphabeticalDescendingStringComparer();
    } else if (_sortType == ESortType::LastLetterAscending) {
        stringSorter = new LastLetterAscendingStringComparer();
    } else {
        throw runtime_error("Sort Type must be one of the following types " + AllSortTypesString);
    }
    while (lists.size() > 1) {
        vector<string> l1 = lists[0];
        vector<string> l2 = lists[1];
        auto v = mergeTwoVector(l1, l2, stringSorter);
        lists.push_back(v);
        lists.erase(lists.begin());
        lists.erase(lists.begin());
    }
    return lists.front();
}

vector<string> mergeTwoVector(vector<string> &o1, vector<string> &o2, IStringComparer *stringComparer) {
    if (o1.empty()) {
        return o2;
    }
    if (o2.empty()) {
        return o1;
    }
    if (stringComparer == nullptr) {
        throw runtime_error("comparer cannot be nullptr");
    }
    vector<string> o3;
    unsigned int i = 0, j = 0;
    while (i < o1.size() && j < o2.size()) {
        if (stringComparer->IsFirstAboveSecond(o1[i], o2[j])) {
            o3.push_back(o1[i]);
            i++;
        } else {
            o3.push_back(o2[j]);
            j++;
        }
    }
    while (i < o1.size()) {
        o3.push_back(o1[i]);
        i++;
    }
    while (j < o2.size()) {
        o3.push_back(o2[j]);
        j++;
    }
    return o3;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteAndPrintResults(const vector<string> &_masterStringList, const string &_outputName, int _clocksTaken) {
    cout << _outputName << "\t- Clocks Taken: " << _clocksTaken << endl;
    std::__fs::filesystem::path cwd = std::__fs::filesystem::current_path() / "MyOutputFiles";
    std::__fs::filesystem::create_directory(cwd);
    ofstream fileOut(cwd.string() + "/" + _outputName + ".txt", ofstream::trunc);
    for (const auto &i: _masterStringList) {
        fileOut << i << endl;
    }
    fileOut.close();
}


