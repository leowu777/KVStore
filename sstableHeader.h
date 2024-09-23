#pragma once

#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>

class sstableHeader {
public:
    static const uint32_t HEADER_SIZE = 32;
    uint64_t timeStamp;
    uint64_t keyCounter;
    uint64_t maxKey;
    uint64_t minKey;
    sstableHeader(uint64_t _t = 0, uint64_t _cnt = 0, uint64_t _max = 0, uint64_t _min = 0);
    void read(const std::string& file, uint32_t startLoc);
    void write(const std::string& file) const;
};
