#include "kvstore.h"
#include <string>
#include <fstream>
#include <vector>

KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog) {
	dir_ = dir;
	vlog_ = vlog;
	skip_list_ = new SkipList(8);
	v_log_ = new VLog(vlog);
	std::vector<std::string> sst_directory_list;
	utils::scanDir(dir_, sst_directory_list);
	for (size_t i = 0; i < sst_directory_list.size(); ++i) {
		std::string level_dir = sst_directory_list[i];
		std::vector<std::string> sst_filename_list;
		utils::scanDir(dir_ + "/" + level_dir, sst_filename_list);
		std::vector<SSTable *> sstable_list;
		for (size_t j = 0; j < sst_filename_list.size(); ++j) {
			std::ifstream in;
			in.open(dir_ + "/" + level_dir + "/" + sst_filename_list[j], std::ios::binary);
			SSTable *sstable = new SSTable(in);
			in.close();
			sstable_list.push_back(sstable);
		}
		sstable_buffer.push_back(sstable_list);
	}
}

KVStore::~KVStore() {
	if (this->skip_list_->get_size() != 0) {
		run_compaction();
	}
	delete this->skip_list_;
	delete this->v_log_;
	for (auto &sstable_list : sstable_buffer) {
        for (auto *sstable : sstable_list) {
        	delete sstable; 
        }
        sstable_list.clear();
    }
    sstable_buffer.clear();
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s) {
	if (this->skip_list_->search(key) != "~SkipListNotFound~") {
		this->skip_list_->insert(key, s);
		return;
	}
	if (this->memTable_need_flush()) {
		run_compaction();
	}
	this->skip_list_->insert(key, s);
}

/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	return "";
}

/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	std::string value = get(key);
	if (value == "") {
		return false;
	} else {
		put(key, "");
		return true;
	}
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list)
{
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the size in byte you should AT LEAST recycle.
 */
void KVStore::gc(uint64_t chunk_size)
{
}

uint32_t KVStore::memTable_need_flush() {
	return 32 + 8192 + (this->skip_list_->get_size() + 1) * sizeof(SSTableTuple) > PAGE_SIZE;
}

uint32_t KVStore::run_compaction() {
	return 0;
}
