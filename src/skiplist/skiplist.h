#ifndef LSM_KV_SkipList_H
#define LSM_KV_SkipList_H

#include <cstdint>
#include <vector>
#include <string>
#include <list>

static const std::string DEL = "~DELETED~";

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
    uint32_t bytes_; // index + value的大小
    const int max_level_;
    int random_level();
public:
    SkipList(int max_level);

    ~SkipList();

    /**
     * insert a new key-value pair
     * @param key
     * @param value
    */
    void insert(uint64_t key, const std::string &value);

    /**
     * try to find the value mapped to key
     * @param key
     * @return The value associated with the key, or an 
     * empty string if the key is not finded, or DEL if 
     * the key is deleted
    */
    std::string search(uint64_t key);

    /**
     * print this list
    */
    void display();

    /**
     * reset
    */
    void reset();

    /**
     * get the current size
    */
    uint32_t get_bytes();
};


#endif //LSM_KV_SkipList_H
