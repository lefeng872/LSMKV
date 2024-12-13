#include "kvstore.h"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog) {
	sstable_dir_path_ = dir;
	vlog_path_ = vlog;
	skip_list_ = new SkipList(8);
	v_log_ = new VLog(vlog_path_);
	std::vector<std::string> sst_directory_list;
	utils::scanDir(sstable_dir_path_, sst_directory_list);
	for (size_t i = 0; i < sst_directory_list.size(); ++i) {
		std::string level_dir = sst_directory_list[i];
		std::vector<std::string> sst_filename_list;
		utils::scanDir(sstable_dir_path_ + "/" + level_dir, sst_filename_list);
		std::vector<SSTable *> sstable_list;
		for (size_t j = 0; j < sst_filename_list.size(); ++j) {
			std::ifstream in;
			in.open(sstable_dir_path_ + "/" + level_dir + "/" + sst_filename_list[j], std::ios::binary);
			SSTable *sstable = new SSTable(in);
			in.close();
			sstable_list.push_back(sstable);
		}
		sstable_buffer.push_back(sstable_list);
	}
	// todo restore global timestamp
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
	// todo
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
	// todo
}

/**
 * Return a list including all the key-value pair between key1 and key2.
 * keys in the list should be in an ascending order.
 * An empty string indicates not found.
 */
void KVStore::scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) {
	// todo
}

/**
 * This reclaims space from vLog by moving valid value and discarding invalid value.
 * chunk_size is the size in byte you should AT LEAST recycle.
 */
void KVStore::gc(uint64_t chunk_size) {
	// todo
}

uint32_t KVStore::memTable_need_flush() {
	return 32 + 8192 + (this->skip_list_->get_size() + 1) * sizeof(SSTableTuple) > PAGE_SIZE;
}

uint32_t KVStore::run_compaction() {
	flush_memTable();
}

void KVStore::flush_memTable() {
	std::vector<std::pair<uint64_t, std::string>> content;
	this->skip_list_->get_content(content);
	// append vlog
	uint64_t start_offset = this->v_log_->append(content);
	// generate sstable
	SSTable *sstable = new SSTable(global_timestamp, start_offset, content);
	global_timestamp++;
	// write sstable
	std::ofstream out;
	out.open(sstable_dir_path_ + "/level-0/" + sstable->get_filename(), std::ios::binary);
	sstable->write_sstable(out);
	out.close();
	// update sstable_buffer
	if (this->sstable_buffer.empty()) {
		std::vector<SSTable *> sstable_list_0;
		sstable_list_0.push_back(sstable);
		this->sstable_buffer.push_back(sstable_list_0);
	} else {
		sstable_buffer.front().push_back(sstable);
	}
	// reset skiplist
	this->skip_list_->clear();
}

void KVStore::merge_sstable_level0() {
	if (this->sstable_buffer.empty()) {
		return;
	}
	std::vector<SSTable *> sstable_collection;
	uint64_t left = UINT64_MAX;
	uint64_t right = 0;
	// collect all sstable in level-0, rm them in buffer and on disk
	for (SSTable *sstable: sstable_buffer.front()) {
		sstable_collection.push_back(sstable);
		if (!utils::rmfile(sstable_dir_path_ + "/level-0/" + sstable->get_filename())) {
			printf("rm level-0 sstable doesn't exist!\n");
		}
		left = std::min(sstable->get_min(), left);
		right = std::max(sstable->get_max(), right);
	}
	sstable_buffer.front().clear();
	// collect overlaped sstable in level-1(if level-1 exists), rm them in buffer and on disk
	if (sstable_buffer.size() > 1) {
		for (auto it = sstable_buffer[1].begin(); it != sstable_buffer[1].end();) {
			if ((*it)->get_min() > right || (*it)->get_max() < left) {
				++it;
			} else {
				sstable_collection.push_back(*it);
				it = sstable_buffer[1].erase(it);
				if (!utils::rmfile(sstable_dir_path_ + "/level-1/" + (*it)->get_filename())) {
					printf("rm level-1 sstable doesn't exist!\n");
				}
			}
		}
	}
	// merge all tuples
	std::sort(sstable_collection.begin(), sstable_collection.end(), [](SSTable *a, SSTable *b) {
		return a->get_timestamp() < b->get_timestamp() || (a->get_timestamp() == b->get_timestamp() && a->get_min() < b->get_min());
	});
	std::vector<SSTableTuple> tuple_collection;
	for (SSTable *sstable: sstable_collection) {
		std::vector<SSTableTuple> sstable_content = sstable->get_content();
		for (SSTableTuple tuple: sstable_content) {
			auto it = tuple_collection.begin();
			while (it != tuple_collection.end()) {
				if ((*it).key < tuple.key) {
					++it;
				} else {
					break;
				}
			}
			if (it != tuple_collection.end() && (*it).key == tuple.key) {
				(*it).offset = tuple.offset;
				(*it).v_len = tuple.v_len;
			} else {
				tuple_collection.insert(it, tuple);
			}
		}
	}
	// break tuple_collection into new sstables
	std::vector<SSTable *> new_table_list;
	uint64_t latest_timestamp = sstable_collection.back()->get_timestamp();
	uint64_t MAX_TUPLE = (PAGE_SIZE - 32 - 8192) / sizeof(SSTableTuple);
	auto it = tuple_collection.begin();
	for (; it + MAX_TUPLE < tuple_collection.end(); it += MAX_TUPLE) {
		std::vector<SSTableTuple> subset(it, it + MAX_TUPLE);
		new_table_list.push_back(new SSTable(latest_timestamp, subset));
	}
	if (it != tuple_collection.end()) {
		std::vector<SSTableTuple> subset(it, tuple_collection.end());
		new_table_list.push_back(new SSTable(latest_timestamp, subset));
	}
	if (sstable_buffer.size() > 1) {
		sstable_buffer[1].insert(sstable_buffer[1].end(), new_table_list.begin(), new_table_list.end());
	} else {
		sstable_buffer.push_back(new_table_list);
	}
	for (SSTable *sstable: new_table_list) {
		std::ofstream out;
		out.open(sstable_dir_path_ + "/level-1/" + sstable->get_filename(), std::ios::binary);
		sstable->write_sstable(out);
		out.close();
	}
}
