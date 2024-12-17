#ifndef LSM_KV_SSTable_H
#define LSM_KV_SSTable_H

#include <vector>
#include <fstream>
#include "../bloomfilter/bloomfilter.h"
#include "../skiplist/skiplist.h"
#include "../vlog/vlog.h"

struct SSTableHeader {
    uint64_t timestamp;
    uint64_t size; // 键值对的数量
    uint64_t min;
    uint64_t max;
};

struct SSTableTuple {
    uint64_t key;
    uint64_t offset; // value在vLog中偏移量
    uint32_t v_len;  // value的长度

    SSTableTuple() {}

    /**
     * Generate a SSTable from MemTable
     */
    SSTableTuple(uint64_t key, uint64_t offset, uint32_t v_len) {
        this->key = key;
        this->offset = offset;
        this->v_len = v_len;
    }
};

struct SSTableTimestampTuple {
    SSTableTuple tuple;
    uint64_t timestamp;
    SSTableTimestampTuple(uint64_t key, uint64_t offset, uint32_t v_len, uint64_t _timestamp) {
        tuple = SSTableTuple(key, offset, v_len);
        this->timestamp = _timestamp;
    }
};

class SSTable {
private:
    SSTableHeader header_;
    BloomFilter filter_;
    std::vector<SSTableTuple> tuple_list_;
public:
    SSTable(uint64_t _timestamp, uint64_t _offset, std::vector<std::pair<uint64_t, std::string>> &content);

    SSTable(std::ifstream &in);

    SSTable(uint64_t _timestamp, const std::vector<SSTableTuple> &content);

    ~SSTable() {}

    std::string get_filename() const;

    uint64_t get_min() const;

    uint64_t get_max() const;

    uint64_t get_timestamp() const;

    void write_sstable(std::ofstream &out);

    /**
     * @param the given vector to fill
     */
    void get_content(std::vector<SSTableTuple> &content) const;

    void scan(uint64_t key1, uint64_t key2, std::vector<SSTableTuple> &result) const;

    /**
     * check bloomfilter for fast reject
     */
    bool check_filter(uint64_t _key) const;

    // binary search, if not found, set _v_len to 0
    bool search(uint64_t _key, uint64_t *_offset, uint32_t *_v_len) const;

    void print() const;
};

#endif //LSM_KV_SSTable_H