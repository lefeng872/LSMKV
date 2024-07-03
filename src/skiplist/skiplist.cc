#include "skiplist.h"
#include <cstdlib>
#include <iostream>

int SkipList::random_level()
{
    int result = 1;
    while (result < max_level_ && ((double)rand() / RAND_MAX) < 0.5) {
        ++result;
    }
    return result;
}

SkipList::SkipList(int max_level): max_level_(max_level) {
    head_ = new SLNode(0, "", TYPE::HEAD, max_level_);
    tail_ = new SLNode(INT_MAX, "", TYPE::TAIL, max_level_);
    for (int i = 0; i < max_level_; ++i) {
        head_->next_list[i] = tail_;
    }
    size_ = 0;
}

SkipList::~SkipList() {
    SLNode *p1 = head_;
    SLNode *p2;
    while (p1) {
        p2 = p1->next_list[0];
        delete p1;
        p1 = p2;
    }
}

void SkipList::insert(uint64_t key, const std::string &value)
{
    SLNode *p = head_;
    std::vector<SLNode *> update(max_level_, nullptr);
    for (int i = max_level_ - 1; i >= 0; --i) {
        while (p->next_list[i]->key < key) {
            p = p->next_list[i];
        }
        update[i] = p;
    }
    p = p->next_list[0];
    if (p->key == key) {
        p->val = value;
    } else {
        size_++;
        int level = random_level();
        p = new SLNode(key, value, NORMAL, max_level_);
        for (int i = 0; i < level; ++i) {
            p->next_list[i] = update[i]->next_list[i];
            update[i]->next_list[i] = p;
        }
    }
}

std::string SkipList::search(uint64_t key)
{
    SLNode *p = head_;
    for (int i = max_level_ - 1; i >= 0; --i) {
        while (p->next_list[i]->key < key) {
            p = p->next_list[i];
        }
    }
    p = p->next_list[0];
    if (p->key == key) {
        return p->val;
    } else {
        return "";
    }
}

void SkipList::display()
{
    for (int i = max_level_ - 1; i >= 0; --i)
    {
        std::cout << "Level " << i + 1 << ":h";
        SLNode *node = head_->next_list[i];
        while (node->type != TYPE::TAIL) {
            std::cout << "-->(" << node->key << "," << node->val << ")";
            node = node->next_list[i];
        }

        std::cout << "-->N" << std::endl;
    }
}

void SkipList::reset() {
    if (head_->next_list[0] == tail_) return;
    SLNode *n1 = head_->next_list[0], *n2;
    while (n1 != tail_) {
        n2 = n1->next_list[0];
        delete n1;
        n1 = n2;
    }
    for (int i = 0; i < max_level_; ++i) {
        head_->next_list[i] = tail_;
    }
    size_ = 0;
}

uint32_t SkipList::get_size() {
    return size_;
}