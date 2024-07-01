#include "skiplist.h"
#include "list"
#include <cstdlib>
#include <iostream>

static const std::string DEL = "~DELETED~";

int skiplist::randomLevel()
{
    int result = 1;
    while (result < MAX_LEVEL && ((double)rand() / RAND_MAX) < 0.5)
    {
        ++result;
    }
    return result;
}

void skiplist::insert(uint64_t key, const std::string &value)
{
    // TODO
    slnode *p = head;
    std::vector<slnode *> update;
    for (int i = 0; i < MAX_LEVEL; ++i)
        update.push_back(nullptr);
    for (int i = MAX_LEVEL - 1; i >= 0; --i) {
        while (p->nxt[i]->key < key)
            p = p->nxt[i];
        update[i] = p;
    }
    p = p->nxt[0];
    if (p->key == key) {
        bytes += value.length();
        bytes -= p->val.length();
        p->val = value;
    } else {
        bytes += 12 + value.length();
        int level = randomLevel();
        p = new slnode(key, value, NORMAL);
        for (int i = 0; i < level; ++i) {
            p->nxt[i] = update[i]->nxt[i];
            update[i]->nxt[i] = p;
        }
    }
}

std::string skiplist::search(uint64_t key)
{
    // TODO
    slnode *p = head;
    for (int i = MAX_LEVEL - 1; i >= 0; --i) {
        while (p->nxt[i]->key < key) {
            p = p->nxt[i];
        }
    }
    p = p->nxt[0];
    if (p->key == key) {
        return p->val;
    } else {
        return "";
    }
}

bool skiplist::del(uint64_t key)
{
    // TODO
    insert(key, DEL);
    return true;
}

void skiplist::Display()
{
    for (int i = MAX_LEVEL - 1; i >= 0; --i)
    {
        std::cout << "Level " << i + 1 << ":h";
        slnode *node = head->nxt[i];
        while (node->type != TYPE::TAIL)
        {
            std::cout << "-->(" << node->key << "," << node->val << ")";
            node = node->nxt[i];
        }

        std::cout << "-->N" << std::endl;
    }
}

uint32_t skiplist::getBytes() {
    return bytes;
}