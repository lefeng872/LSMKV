#ifndef LSM_KV_SKIPLIST_H
#define LSM_KV_SKIPLIST_H

#include <cstdint>
#include <vector>
#include <string>
#include <list>

const int MAX_LEVEL = 18;

enum TYPE
{
    HEAD = 1,
    NORMAL,
    TAIL
};

struct slnode
{
    uint64_t key;
    std::string val;
    TYPE type;
    std::vector<slnode *> nxt;
    slnode(uint64_t _key, std::string _val, TYPE _type)
        : key(_key), val(_val), type(_type)
    {
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            nxt.push_back(nullptr);
        }
    }
};

class skiplist {
private:
    slnode *head;
    slnode *tail;
    int randomLevel();
    uint32_t bytes; // index + value的大小
public:
    skiplist(double p)
    {
        head = new slnode(0, "", TYPE::HEAD);
        tail = new slnode(INT_MAX, "", TYPE::TAIL);
        for (int i = 0; i < MAX_LEVEL; ++i)
        {
            head->nxt[i] = tail;
        }
        bytes = 0;
    }

    void insert(uint64_t key, const std::string &value);

    std::string search(uint64_t key); // delete也被当成普通串

    bool del(uint64_t key);

    void Display();

    void reset() {
        if (head->nxt[0] == tail) return;
        slnode *n1 = head->nxt[0], *n2;
        while (n1 != tail) {
            n2 = n1->nxt[0];
            delete n1;
            n1 = n2;
        }
        for (int i = 0; i < MAX_LEVEL; ++i) {
            head->nxt[i] = tail;
        }
        bytes = 0;
    }

    ~skiplist()
    {
        slnode *n1 = head;
        slnode *n2;
        while (n1)
        {
            n2 = n1->nxt[0];
            delete n1;
            n1 = n2;
        }
    }

    uint32_t getBytes();

    slnode* getFirst(){return head->nxt[0];}
};


#endif //LSM_KV_SKIPLIST_H
