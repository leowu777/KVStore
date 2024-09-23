#include "sstableHeader.h"

sstableHeader::sstableHeader(uint64_t _t, uint64_t _cnt, uint64_t _max, uint64_t _min): 
                             timeStamp(_t), keyCounter(_cnt), maxKey(_max), minKey(_min) {}

void sstableHeader::read(const std::string& file, uint32_t startLoc) {
    std::ifstream f;
    f.open(file, std::ios_base::in | std::ios_base::binary);
    f.seekg(startLoc, std::ios_base::beg);
    f.read((char*)&timeStamp, sizeof(uint64_t));
    f.read((char*)&keyCounter, sizeof(uint64_t));
    f.read((char*)&maxKey, sizeof(uint64_t));
    f.read((char*)&minKey, sizeof(uint64_t));
    f.close();
    return;
}

void sstableHeader::write(const std::string& file) const {
    std::ofstream f;
    f.open(file, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    f.write((char*)&timeStamp, sizeof(uint64_t));
    f.write((char*)&keyCounter, sizeof(uint64_t));
    f.write((char*)&maxKey, sizeof(uint64_t));
    f.write((char*)&minKey, sizeof(uint64_t));
    f.close();
    return;
}
