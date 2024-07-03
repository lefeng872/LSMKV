#ifndef LSM_KV_SSTable_H
#define LSM_KV_SSTable_H

#include <vector>
#include "../bloomfilter"

struct SSTableHeader {
    uint64_t timestamp;
    uint64_t size;
    uint64_t min;
    uint64_t max;
};

struct SSTableTuple {
    uint64_t key;
    uint64_t offset;
    uint32_t v_len;
};

class SSTable {
private:
    SSTableHeader header;
    BloomFilter filter;
    std::vector<SSTableTuple> tuple_list;
public:
    // 二分查找搜索key，通过offset从vLog文件读取键值对
    // 将SSTable缓存在内存中
};

#endif //LSM_KV_SSTable_H