#include "bloomFilter.h"
#include "MurmurHash3.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>


bloomFilter::bloomFilter(): bitset(nullptr) {}

bloomFilter::bloomFilter(bloomFilter &&other) noexcept {
    bitset = other.bitset;
    other.bitset = nullptr;
}

bloomFilter::~bloomFilter() {
    if(bitset) {
        free(bitset);
        bitset = nullptr;
    }
}

void bloomFilter::initialize() {
    if(!bitset) {
        bitset = malloc(FILTER_SIZE);
        if(!bitset) {
            std::cout << "Fail to malloc for bloom filter" << std::endl;
            exit(1);
        }
        memset(bitset, 0, FILTER_SIZE);
    }
}

void bloomFilter::add(uint64_t key) {
    if(!bitset) return;
    // 四个uint32_t
    void *tmp = malloc(16);
    if(!tmp) {
        std::cout << "Fail to malloc for bloom filter" << std::endl;
        exit(1);
    }
    MurmurHash3_x64_128(&key, 8, 1, tmp);
    for(int i = 0;i < 4;++ i) {
        uint32_t pos = ((uint32_t*)tmp)[i] % (FILTER_SIZE << 3);
        uint32_t byteCnt = pos >> 3;
        uint32_t bitCnt = pos & 0b111;
        (((uint8_t*)bitset)[byteCnt]) |= (1 << bitCnt);
    }
    free(tmp);
    return;
}

bool bloomFilter::ifExist(uint64_t key) const {
    if(!bitset) return 0;
    void *tmp = malloc(16);
    if(!tmp) {
        std::cout << "Fail to malloc for bloom filter" << std::endl;
        exit(1);
    }
    MurmurHash3_x64_128(&key, 8, 1, tmp);
    for(int i = 0;i < 4;++ i) {
        uint32_t pos = ((uint32_t*)tmp)[i] % (FILTER_SIZE << 3);
        uint32_t byteCnt = pos >> 3;
        uint32_t bitCnt = pos & 0b111;
        // 出现0位
        if(!(((uint8_t*)bitset)[byteCnt] & (1 << bitCnt))) {
            free(tmp);
            return 0;
        }
    }
    free(tmp);
    return 1;
}

void bloomFilter::read(const std::string &file, uint32_t startLoc) {
    std::ifstream f;
    f.open(file, std::ios_base::in | std::ios_base::binary);
    if(!f.is_open()) {
        std::cout << "Fail to open file in path: " + file << std::endl;
        exit(1);
    }
    if(!bitset) {
        bitset = malloc(FILTER_SIZE);
        if(!bitset) {
            std::cout << "Fail to malloc for bloom filter" << std::endl;
            exit(1);
        }
    }
    f.seekg(startLoc, std::ios_base::beg);
    f.read((char*)bitset, FILTER_SIZE);
    if(!f.good()) {
        std::cout << "Fail to read at location: " + std::to_string(startLoc) << std::endl;
        exit(1);
    }
    f.close();
    return;
}

void bloomFilter::write(const std::string &file) const {
    if(!bitset) return;
    std::ofstream f;
    f.open(file, std::ios_base::out | std::ios_base::binary | std::ios_base::app);
    if(!f.is_open()) {
        std::cout << "Fail to open file in path: " + file << std::endl;
        exit(1);
    }
    f.write((char*)bitset, FILTER_SIZE);
    if(!f.good()) {
        std::cout << "Fail to write in file: " + file << std::endl;
        exit(1);
    }
    f.close();
    return;
}
