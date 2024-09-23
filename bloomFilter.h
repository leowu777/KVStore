#pragma once

#include <cstdint>
#include <string>

class bloomFilter {
private:
    void *bitset;
public:
    const static uint64_t FILTER_SIZE = 10240;

    bloomFilter();
    bloomFilter(const bloomFilter&) = delete;
    bloomFilter &operator=(const bloomFilter&) = delete;
    // 移动构造函数, 避免深拷贝带来的性能浪费
    bloomFilter(bloomFilter &&) noexcept;
    ~bloomFilter();
    void initialize();
    void add(uint64_t key);
    bool ifExist(uint64_t key) const;
    void read(const std::string &file, uint32_t startLoc);
    void write(const std::string &file) const;
};