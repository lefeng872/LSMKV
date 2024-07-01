#include "BloomFilter.h"

void BloomFilter::insert(uint64_t key) {
    MurmurHash3_x64_128(&key, sizeof (key), 1, hashV);
    for (int i = 0; i < 4; ++i) {
        uint32_t p = (hashV[i] % (8 * M));
        s.set(p, true);
    }
}

bool BloomFilter::search(uint64_t key) {
    MurmurHash3_x64_128(&key, sizeof(key), 1, hashV);
    for (int i = 0; i < 4; ++i) {
        uint32_t p = (hashV[i] % (8 * M));
        if (!s[p]) return false;
    }
    return true;
}