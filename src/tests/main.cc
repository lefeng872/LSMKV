#include "../kvstore/kvstore.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include "../utils/utils.h"
#include <string>
#include <fstream>

#define TEST_MAX 1024

void print_sstable(const std::string &filename) {
    std::ifstream in;
    in.open(filename, std::ios::binary);
    SSTable *sstable = new SSTable(in);
    in.close();
    sstable->print();
}

void print_vlog(const std::string &filename) {
    VLog vlog(filename);
    vlog.print();
}

void learn_read() {
    std::ifstream in;
    in.open("data/vlog/vlog.db", std::ios::binary);
    uint64_t tail = utils::seek_data_block("data/vlog/vlog.db");
    in.seekg(tail);
    uint8_t byte = 0;
    while (!in.read(reinterpret_cast<char *> (&byte), 1).eof()) {
        printf("read byte=%hhu\n", byte);
        if (byte == 0xff) {
            break;
        }
    }
    // printf("Jumped out\n");
}

int main() {
    // print_sstable("data/sstables/level-0/0-339_0.sst");
    // print_vlog("data/vlog/vlog.db"); 
    // learn_read();
    KVStore store("./data/sstables", "./data/vlog/vlog.db");
    // for (uint32_t i = 0; i < TEST_MAX; ++i) {
    //     printf("insert [%u, s%u]\n", i, i);
    //     std::string value = "s" + std::to_string(i);
    //     store.put(i, value);
    // }
    // printf("At least insert ok\n");
    // for (uint32_t i = 0; i < TEST_MAX; ++i) {
    //     std::string value = store.get(i);
    //     printf("search [%u, %s]\n", i, value.c_str());
    // }
    store.reset(); 
}