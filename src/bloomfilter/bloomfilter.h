#ifndef LSM_KV_BloomFilter_H
#define LSM_KV_BloomFilter_H

#include "../utils/MurmurHash3.h"
#include <cstdint>
#include <bitset>

const uint32_t FILTER_MAX = 8192;

class BloomFilter {
public:
    std::bitset<8 * FILTER_MAX> bitset_;
    uint32_t hashV[4];
public:
    BloomFilter() {}
    ~BloomFilter() {}
    void reset() {bitset_.reset();}
    void insert(uint64_t key);
    bool search(uint64_t key);
};

#endif //LSM_KV_BloomFilter_H