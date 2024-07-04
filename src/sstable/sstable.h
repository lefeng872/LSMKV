#ifndef LSM_KV_SSTable_H
#define LSM_KV_SSTable_H

#include <vector>
#include "../bloomfilter/bloomfilter.h"

struct SSTableHeader {
    uint64_t timestamp;
    uint64_t size; // 键值对的数量
    uint64_t min;
    uint64_t max;
};

struct SSTableTuple {
    uint64_t key;
    uint64_t offset; // value在vLog中偏移量
    uint32_t v_len;  // 长度
};

class SSTable {
private:
    SSTableHeader header_;
    BloomFilter filter_;
    std::vector<SSTableTuple> tuple_list_;
public:
    SSTable() {
        header_.timestamp = -1;
        header_.size = -1;
        header_.min = -1;
        header_.max = -1;
        filter_.reset();
    }

    // 二分查找搜索key，通过offset从vLog文件读取键值对
    // 将SSTable缓存在内存中
};

#endif //LSM_KV_SSTable_H