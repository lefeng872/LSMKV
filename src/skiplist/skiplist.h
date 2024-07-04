#ifndef LSM_KV_SkipList_H
#define LSM_KV_SkipList_H

#include <cstdint>
#include <climits>
#include <vector>
#include <string>
#include <list>
#include <utility>

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
    uint32_t size_; // list里pair的数量
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
     * get the num of key value pairs
    */
    uint32_t get_size();

    /**
     * @brief output the content into a sorted vector
     * @param content
    */
    void get_content(std::vector<std::pair<uint64_t, std::string>> &content);
};


#endif //LSM_KV_SkipList_H
