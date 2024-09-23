#pragma once

#include <string>
#include <vector>
#include "sstableHeader.h"
#include "bloomFilter.h"

class ssTable {
public:
    sstableHeader header;
    bloomFilter filter;
    std::pair<uint64_t, uint32_t> *indSeq;
    std::string file;
    int64_t binarySearch(uint64_t key, uint32_t l, uint32_t r) const;
    std::vector<uint64_t> getKeySeq ();
    std::string getValInd(uint32_t ind) const;
    ssTable(): indSeq(nullptr) {}
    ~ssTable();
    // 为提高效率，使用移动构造
    ssTable(const ssTable &) = delete;
    ssTable &operator = (const ssTable &) = delete;
    ssTable(ssTable &&) noexcept;
    void read(const std::string &file);
    std::pair<bool, std::string> getValKey(uint64_t key) const;
    void clearTable();
    static ssTable* genTable(const std::string &file, uint64_t timeStamp, const std::vector<std::pair<uint64_t, std::string>> &kVarray);
};


