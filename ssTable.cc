#include <fstream>
#include <iostream>
#include "ssTable.h"
#include "utils.h"

int64_t ssTable::binarySearch(uint64_t key, uint32_t l, uint32_t r) const {
    if (!indSeq || l > r)  return -1;
    uint32_t m = (l + r) >> 1;
    if (indSeq[m].first == key)  return m;
    if (indSeq[m].first < key)  return binarySearch(key, m + 1, r);
    return binarySearch(key, l, m - 1);
}

ssTable::ssTable(ssTable &&other) noexcept: filter(std::move(other.filter)){
    indSeq = other.indSeq;
    other.indSeq = nullptr;
}


ssTable::~ssTable() {
    delete [] indSeq;
    indSeq = nullptr;
}

void ssTable::read(const std::string &filePath) {
    file = filePath;
    header.read(file, 0);
    filter.read(file, sstableHeader::HEADER_SIZE);
    indSeq = new std::pair<uint64_t, uint32_t> [header.keyCounter];
    std::ifstream f;
    f.open(file, std::ios_base::out | std::ios_base::binary);
    f.seekg(sstableHeader::HEADER_SIZE + bloomFilter::FILTER_SIZE, std::ios_base::beg);
    for (uint64_t i = 0; i < header.keyCounter; ++i) {
        f.read((char *) &(indSeq[i].first), sizeof (uint64_t));
        f.read((char *) &(indSeq[i].second), sizeof (uint32_t));
    }
    f.close();
}

std::pair<bool, std::string> ssTable::getValKey(uint64_t key) const {
    std::pair<bool, std::string> res(false, "");
    if (indSeq == nullptr || key < header.minKey || key > header.maxKey || !filter.ifExist(key)) {
        return res;
    }
    int64_t test = binarySearch(key, 0, header.keyCounter - 1);
    if (test == -1) {
        return res;
    }
    std::ifstream f;
    f.open(file, std::ios_base::out | std::ios_base::binary);
    uint32_t loffset = (indSeq[test]).second;
    uint32_t roffset;
    if (uint64_t (test) == header.keyCounter - 1) {
        f.seekg(0, std::ios_base::end);
        roffset = f.tellg();
    } else {
        roffset = (indSeq[test + 1]).second;
    }
    res.first = true;
    char *tmp = new char [roffset - loffset];
    f.seekg(loffset, std::ios_base::beg);
    f.read(tmp, static_cast<int32_t>((roffset - loffset) * sizeof (char)));
    res.second = std::string(tmp, roffset - loffset);
    delete [] tmp;
    f.close();
    return res;
}

void ssTable::clearTable() {
    utils::rmfile(file.c_str());
    delete [] indSeq;
    indSeq = nullptr;
}

ssTable* ssTable::genTable(const std::string &file, uint64_t timeStamp, const std::vector<std::pair<uint64_t, std::string>> &KVSeq) {
    auto newTable = new ssTable;
    newTable->filter.initialize();
    std::ofstream f;
    uint64_t &maxKey = newTable->header.maxKey;
    uint64_t &minKey = newTable->header.minKey;
    uint64_t &keyCounter = newTable->header.keyCounter;
    maxKey = minKey = KVSeq.cbegin()->first;
    keyCounter = 0;
    for(auto it = KVSeq.cbegin();it !=KVSeq.cend();++ it) {
        ++ keyCounter;
        if (it->first > maxKey)  maxKey = it->first;
        if (it->first < minKey)  minKey = it->first;
        newTable->filter.add(it->first);
    }
    newTable->header.timeStamp = timeStamp;
    newTable->header.write(file);
    newTable->filter.write(file);
    f.open(file, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    uint32_t offset = sstableHeader::HEADER_SIZE + bloomFilter::FILTER_SIZE + keyCounter * (sizeof (uint32_t) + sizeof (uint64_t));
    newTable->indSeq = new std::pair<uint64_t, uint32_t> [keyCounter];
    uint64_t i = 0;
    for(auto it = KVSeq.cbegin();it != KVSeq.cend();++ it) {
        f.write((char *) &(it->first), sizeof (uint64_t));
        f.write((char *) &offset, sizeof (uint32_t));
        newTable->indSeq[i++] = std::pair<uint64_t, uint32_t> (it->first, offset);
        offset += it->second.length() * sizeof (char);
    }
    newTable->file = file;
    for(auto it = KVSeq.cbegin();it != KVSeq.cend();++ it) {
        f.write(it->second.c_str(), static_cast<int32_t>(it->second.length() * sizeof (char)));
    }
    f.close();
    return newTable;
}

std::vector<uint64_t> ssTable::getKeySeq() {
    std::vector<uint64_t> res;
    if (indSeq == nullptr) {
        return res;
    }
    for (uint32_t i = 0; i < header.keyCounter; ++i) {
        res.emplace_back(indSeq[i].first);
    }
    return res;
}

std::string ssTable::getValInd(uint32_t ind) const {
    std::string res;
    std::ifstream f;
    f.open(file, std::ios_base::out | std::ios_base::binary);
    uint32_t loffset = (indSeq[ind]).second;
    uint32_t roffset;
    if (ind == header.keyCounter - 1) {
        f.seekg(0, std::ios_base::end);
        roffset = f.tellg();
    } else {
        roffset = (indSeq[ind + 1]).second;
    }
    char *tmp = new char [roffset - loffset];
    f.seekg(loffset, std::ios_base::beg);
    f.read(tmp, ((roffset - loffset) * sizeof (char)));
    res = std::string(tmp).substr(0, roffset - loffset);
    delete [] tmp;
    f.close();
    return res;
}


