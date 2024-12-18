#include "../kvstore/kvstore.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include "../utils/utils.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <semaphore.h>
#include <random>
#include <signal.h>
#include "test.h"

const uint64_t TEST_MAX = 1024 * 32;
const uint64_t GC_TRIGGER = 1024;

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
    
		uint64_t i;

		// Clean up
		store.reset();

		// Test multiple key-value pairs
		for (i = 0; i < TEST_MAX; ++i)
		{
			store.put(i, std::string(i + 1, 's'));
		}

		// Test deletions
		for (i = 0; i < TEST_MAX; i += 2)
		{
			store.del(i);

			if ((i / 2) % 1024 == 0) [[unlikely]]
			{
				store.gc(16 * MB);
			}
		}

		// Prepare data for Test Mode
		for (i = 0; i < TEST_MAX; ++i)
		{
			switch (i & 3)
			{
			case 0:
				if ("" != store.get(i)) {
                    printf("wrong\n");
                }
				store.put(i, std::string(i + 1, 't'));
				break;
			case 1:
                if (std::string(i + 1, 's') != store.get(i)) {
                    printf("wrong\n");
                }
				store.put(i, std::string(i + 1, 't'));
				break;
			case 2:
                if ("" != store.get(i)) {
                    printf("wrong\n");
                }
				break;
			case 3:
                if (std::string(i + 1, 's') != store.get(i)) {
                    printf("worng\n");
                }
				break;
			default:
				assert(0);
			}

			if (i % GC_TRIGGER == 0) [[unlikely]]
			{
			    store.gc(8 * MB);
			}
		}

		store.gc(32 * MB);
}