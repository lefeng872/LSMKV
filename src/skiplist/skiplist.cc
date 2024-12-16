#include "skiplist.h"
#include <cstdlib>
#include <iostream>

int SkipList::random_level() {
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

void SkipList::insert(uint64_t key, const std::string &value) {
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

std::string SkipList::search(uint64_t key) const {
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
        return "~SkipListNotFound~";
    }
}

void SkipList::display() const {
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

void SkipList::print_pair() const {
    SLNode *node = head_->next_list[0];
    while (node->type != TYPE::TAIL) {
        std::cout << "-->(" << node->key << "," << node->val.substr(0, 5) << ")";
        node = node->next_list[0];
    }
    std::cout << "-->N" << std::endl;
}

void SkipList::clear() {
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

uint32_t SkipList::get_size() const {
    return size_;
}

void SkipList::get_content(std::vector<std::pair<uint64_t, std::string>> &content) const {
    SLNode *p = head_;
    while (p->next_list[0] != tail_) {
        p = p->next_list[0];
        content.push_back(std::make_pair(p->key, p->val));
    }
}