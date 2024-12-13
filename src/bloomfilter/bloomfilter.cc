#include "bloomfilter.h"

void BloomFilter::insert(uint64_t key) {
    MurmurHash3_x64_128(&key, sizeof (key), 1, hashV);
    for (int i = 0; i < 4; ++i) {
        uint32_t p = (hashV[i] % (8 * FILTER_MAX));
        bitset_.set(p, true);
    }
}

bool BloomFilter::search(uint64_t key) const {
    MurmurHash3_x64_128(&key, sizeof(key), 1, hashV);
    for (int i = 0; i < 4; ++i) {
        uint32_t p = (hashV[i] % (8 * FILTER_MAX));
        if (!bitset_[p]) return false;
    }
    return true;
}
