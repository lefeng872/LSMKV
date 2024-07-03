#ifndef LSM_KV_BloomFilter_H
#define LSM_KV_BloomFilter_H

#include "../utils/MurmurHash3.h"
#include <cstdint>
#include <bitset>

const uint32_t M = 8192;

class BloomFilter {
private:
    std::bitset<8 * M> s;
    uint32_t hashV[4];
public:
    BloomFilter() {}
    ~BloomFilter() {}
    void reset() {s.reset();}
    void insert(uint64_t key);
    bool search(uint64_t key);
};

#endif //LSM_KV_BloomFilter_H