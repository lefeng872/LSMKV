#include "kvstore.h"
#include <string>
#include <filesystem>
#include <fstream>

uint32_t KVStore::get_skip_list_bytes(SkipList *skip_list) {
	return 32 + 8192 + skip_list->get_size() * sizeof(SSTableTuple);
}

uint32_t KVStore::run_compaction() {
	return 0;
}

void clear_directory(const std::filesystem::path& dir_path) {
    try {
        if (!std::filesystem::exists(dir_path)) {
			std::filesystem::create_directory(dir_path);
            return;
        }
        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            std::filesystem::remove(entry);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // who cares what you have to say...
    }
}

void KVStore::flush_sstable(uint32_t level) {
	for (int i = 0; i <= level; ++i) {
		std::filesystem::path directory_path = "/data/sstable/level-" + std::to_string(i);
        clear_directory(directory_path);
		for (int j = 0; j < sstable_list_[i].size(); ++j) {
			std::string filename = "/data/sstable/level-" + std::to_string(i) + "/" + std::to_string(j) + ".sst";
			std::ofstream file(filename);
			if (file.is_open()) {
            	SSTable *table = sstable_list_[i][j];
				// todo
            	file.close();
        	}
		}
	}
	return;
}

KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog) {
	skip_list_ = new SkipList(8);
}

KVStore::~KVStore()
{
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s) {
	if (get_skip_list_bytes(this->skip_list_) + s.length() <= 16 * KB) {
		skip_list_->insert(key, s);
	} else {
		SSTable *sstable = new SSTable();
		sstable->flush_skip_list(skip_list_, v_log_);
		sstable->set_timestamp(this->global_timestamp);
		this->global_timestamp++;
		sstable_list_[0].push_back(sstable);
		uint32_t level = 0;
		if (sstable_list_[0].size() > 2) {
			level = run_compaction();
		}
		flush_sstable(level);
		this->skip_list_->reset();
		this->skip_list_->insert(key, s);
	}
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
		put(key, DEL);
		return true;
	}
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
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