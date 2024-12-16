#include "../kvstore/kvstore.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include "../utils/utils.h"
#include <string>
#include <fstream>

#define TEST_MAX 1000

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
    // print_sstable("data/sstables/level-1/340-511_3.sst");
    // print_vlog("data/vlog/vlog.db"); 
    // learn_read();
    KVStore store("./data/sstables", "./data/vlog/vlog.db");
    for (uint64_t i = 0; i < TEST_MAX; ++i) {
        store.put(i, std::string(i + 1, 's'));
    }
    printf("Above is after all insert \n");
    for (uint64_t i = 0; i < TEST_MAX; i += 2) {
        store.del(i);
    }
    for (uint64_t i = 0; i < TEST_MAX; ++i) {
        if ((1 & i) != store.del(i)) {
            printf("******del [%lu] error*******\n", i);
        } else {
            printf("del[%lu] is alright\n", i);
        }
    }
    // store.reset(); 
}