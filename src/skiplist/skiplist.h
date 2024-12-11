#ifndef LSM_KV_SkipList_H
#define LSM_KV_SkipList_H

#include <cstdint>
#include <climits>
#include <vector>
#include <string>
#include <list>
#include <utility>

enum TYPE {
    HEAD = 1,
    NORMAL,
    TAIL
};

struct SLNode {
    uint64_t key;
    std::string val;
    TYPE type;
    std::vector<SLNode *> next_list;
    SLNode(uint64_t key, std::string val, TYPE type, int level)
        : key(key), val(val), type(type), next_list(level, nullptr) {}
};

class SkipList {
private:
    SLNode *head_;
    SLNode *tail_;
    uint32_t size_; // list里pair的数量
    const int max_level_;
    int random_level();
public:
    /**
     * set a new key-value pair,
     * if key already exists, update value,
     * else create new pair.
     * @param key
     * @param value
    */
    void insert(uint64_t key, const std::string &value);

    /**
     * clear all data and reset size to 0
    */
    void clear();

    /**
     * get the num of key value pairs
     * @return size
    */
    uint32_t get_size() const;

    /**
     * get the value mapped to key, if this key is not in list, return "~SkipListNotFound~"
     * @param key
     * @return The value map to the given key
    */
    std::string search(uint64_t key) const;

    /**
     * print this list
    */
    void display() const;

    /**
     * output the content into a sorted vector
     * @param content the vector to be filled
    */
    void get_content(std::vector<std::pair<uint64_t, std::string>> &content) const;

    SkipList(int max_level);

    ~SkipList();

};


#endif //LSM_KV_SkipList_H
