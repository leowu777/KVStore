#pragma once

#include <string>
#include <deque>
#include <vector>
#include "ssTable.h"
struct entryForMerge {
    uint64_t key;
    uint32_t ind;
    ssTable *table;
    entryForMerge(uint64_t _key = 0, uint32_t _ind = 0, ssTable *t = nullptr): key(_key), ind(_ind), table(t) {}
};
struct entryForCmp {
    uint64_t timeStamp;
    uint64_t minKey;
    uint64_t maxKey;
    uint32_t index;
    ssTable *table;
    entryForCmp(uint64_t t, uint64_t mi, uint64_t ma, uint64_t i, ssTable *c)
        : timeStamp(t), minKey(mi), maxKey(ma), index(i), table(c) {}
    bool operator > (const entryForCmp& other) const {
        return timeStamp > other.timeStamp || (timeStamp == other.timeStamp && minKey > other.minKey);
    }
};

class ssTableCache {
public:
    static constexpr const char* DELETE_FLAG = "~DELETED~";
    std::string filePath;
    std::vector<std::deque<ssTable *>> levelArray;
    std::vector<uint64_t> nextLabel;
    uint64_t nextTimeStamp;
    static inline uint64_t MAX_SIZE(uint64_t level) { return 0b10 << level; }
    static std::vector<entryForMerge> genSortedEntry(std::vector<uint64_t> &&keyArray, ssTable *table);
    static void mergesort(std::vector<entryForMerge> &array, uint64_t start, uint64_t end);
    void compaction(uint32_t targetLevel, std::vector<ssTable *> &targetCells);
    void modify();
    explicit ssTableCache(std::string f);
    ~ssTableCache();
    void addTable(const std::vector<std::pair<uint64_t, std::string>> &kVarray);
    std::pair<bool, std::string> getValKey(uint64_t key) const;
    void reset();
};



