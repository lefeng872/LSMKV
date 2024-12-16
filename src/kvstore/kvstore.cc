#include "kvstore.h"
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdint>

KVStore::KVStore(const std::string &dir, const std::string &vlog) : KVStoreAPI(dir, vlog) {
	this->sstable_dir_path_ = dir;
	this->vlog_path_ = vlog;
	skip_list_ = new SkipList(8);
	v_log_ = new VLog(vlog_path_);
	global_timestamp = 0;
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
			global_timestamp = std::max(sstable->get_timestamp(), global_timestamp);
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
std::string KVStore::get(uint64_t key) const
{
	std::string value = this->skip_list_->search(key);
	uint64_t matched_timestamp = 0;
	bool find_flag = false;
	uint64_t offset = 0;
	uint32_t v_len = 0;
	if (value == "~SkipListNotFound~") {
		for (auto level_sstable_list: sstable_buffer) {
			for (SSTable *sstable: level_sstable_list) {
				if (sstable->get_min() <= key && 
					sstable->get_max() >= key && 
					sstable->check_filter(key) && 
					sstable->get_timestamp() >= matched_timestamp) {
					if (sstable->search(key, &offset, &v_len)) {
						find_flag = true;
						matched_timestamp = sstable->get_timestamp();
					}
				}
			}
			if (v_len) {
				return this->v_log_->read_value(offset, v_len);
			} else if (!v_len && find_flag) {
				return "";
			}
		}
		return "";
	} else {
		return value;
	}
}

/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key) {
		// printf("Del[%lu]\n", key);
		// printf("========before del:\n");
		// print_memtable();
		// print_sstable_buffer();
	std::string value = get(key);
	bool flag;
	if (value == "") {
		flag = false;
	} else {
		put(key, "");
		flag = true;
	}
		// printf("=========after del:\n");
		// print_memtable();
		// print_sstable_buffer();
	return flag;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset() {
	this->skip_list_->clear();
	this->v_log_->reset();
	for (auto level_sstable_list: this->sstable_buffer) {
		for (SSTable *sstable: level_sstable_list) {
			delete sstable;
		}
		level_sstable_list.clear();
	}
	this->sstable_buffer.clear();
	std::vector<std::string> sst_directory_list;
	utils::scanDir(this->sstable_dir_path_, sst_directory_list);
	for (std::string level_dir: sst_directory_list) {
		std::vector<std::string> sst_file_list;
		utils::scanDir(sstable_dir_path_ + "/" + level_dir, sst_file_list);
		for (std::string sst_filename: sst_file_list) {
			utils::rmfile(sstable_dir_path_ + "/" + level_dir + "/" + sst_filename);
		}
		utils::rmdir(sstable_dir_path_ + "/" + level_dir);
	}
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

void KVStore::print() const {
	print_memtable();
	print_sstable_buffer();
}

uint32_t KVStore::memTable_need_flush() {
	return 32 + 8192 + (this->skip_list_->get_size() + 1) * sizeof(SSTableTuple) > SSTABLE_MAX_SIZE;
}

void KVStore::run_compaction() {
	// printf("=============This is huge!!!!!!!!!!!, before compaction:\n");
	// print();

	flush_memTable();
	if (!sstable_buffer.empty() && sstable_buffer[0].size() > level_max_sstable_num(0)) {
		merge_sstable_level0();
	}
	for (uint32_t level = 1; level < sstable_buffer.size(); ++level) {
		if (sstable_buffer[level].size() > level_max_sstable_num(level)) {
			merge_sstable_levelx(level);
		} else {
			break;
		}
	}

	// printf("=============After the compaction!!!!!!!!!\n");
	// print();
}

void KVStore::flush_memTable() {
	if (!this->skip_list_->get_size()) {
		return;
	}
	std::vector<std::pair<uint64_t, std::string>> content;
	this->skip_list_->get_content(content);
	// append vlog
	uint64_t start_offset = this->v_log_->append(content);
	// generate sstable
	SSTable *sstable = new SSTable(global_timestamp, start_offset, content);
	global_timestamp++;
	// write sstable
	if (!utils::dirExists(sstable_dir_path_ + "/level-0")) {
		utils::mkdir(sstable_dir_path_ + "/level-0");
	}
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
	// printf("===============Start=================\n");
	if (this->sstable_buffer.empty()) {
		return;
	}
	std::vector<SSTable *> sstable_collection;
	uint64_t left = UINT64_MAX;
	uint64_t right = 0;
	// collect all sstable in level-0, rm them in buffer and on disk
	for (SSTable *sstable: sstable_buffer.front()) {
		// printf("remove [%s]\n", sstable->get_filename().c_str());
		// print_sstable_buffer();
		// print_sstable_disk();
		sstable_collection.push_back(sstable);
		if (utils::rmfile(sstable_dir_path_ + "/level-0/" + sstable->get_filename())) {
			printf("rm level-0 sstable[%s] doesn't exist!\n", sstable->get_filename().c_str());
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
				// printf("remove [%s]\n", (*it)->get_filename().c_str());
				// print_sstable_buffer();
				// print_sstable_disk();
				if (utils::rmfile(sstable_dir_path_ + "/level-1/" + (*it)->get_filename())) {
					printf("rm level-1 sstable[%s] doesn't exist!_____________________________\n", (*it)->get_filename().c_str());
				}
				sstable_collection.push_back(*it);
				it = sstable_buffer[1].erase(it);
			}
		}
	}
	// merge all tuples
	std::vector<SSTableTuple> tuple_collection;
	uint64_t latest_timestamp = merge_sort_sstable(sstable_collection, tuple_collection);
	// break tuple_collection into new sstables
	std::vector<SSTable *> new_table_list;
	uint64_t MAX_TUPLE = (SSTABLE_MAX_SIZE - 32 - 8192) / sizeof(SSTableTuple);
	auto it = tuple_collection.begin();
	for (; it + MAX_TUPLE <= tuple_collection.end(); it += MAX_TUPLE) {
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
	if (!utils::dirExists(sstable_dir_path_ + "/level-1")) {
		utils::mkdir(sstable_dir_path_ + "/level-1");
	}
	for (SSTable *sstable: new_table_list) {
		std::ofstream out;
		out.open(sstable_dir_path_ + "/level-1/" + sstable->get_filename(), std::ios::binary);
		sstable->write_sstable(out);
		out.close();
	}
	for (SSTable *sstable: sstable_collection) {
		delete sstable;
	}
}

void KVStore::merge_sstable_levelx(uint32_t level) {
	if (this->sstable_buffer.size() <= level || this->sstable_buffer[level].size() <= level_max_sstable_num(level)) {
		return;
	}
	std::vector<SSTable *> sstable_collection;
	uint64_t left = UINT64_MAX;
	uint64_t right = 0;
	std::sort(sstable_buffer[level].begin(), sstable_buffer[level].end(), [](SSTable *a, SSTable *b) {
		return a->get_timestamp() < b->get_timestamp() || (a->get_timestamp()  == b->get_timestamp() && a->get_min() < b->get_min());
	});
	uint32_t n_remove = sstable_buffer[level].size() - level_max_sstable_num(level);
	sstable_collection.insert(sstable_collection.end(), sstable_buffer[level].begin(), sstable_buffer[level].begin() + n_remove);
	sstable_buffer[level].erase(sstable_buffer[level].begin(), sstable_buffer[level].begin() + n_remove);
	for (SSTable *sstable: sstable_collection) {
		left = std::min(left, sstable->get_min());
		right = std::max(right, sstable->get_max());
		if (utils::rmfile(sstable_dir_path_ + "/level-" + std::to_string(level) +"/" + sstable->get_filename())) {
			printf("rm level-%u sstable[%s] doesn't exist!_________\n", level, sstable->get_filename().c_str());
		}
	}
	if (sstable_buffer.size() > level + 1) {
		for (auto it = sstable_buffer[level + 1].begin(); it != sstable_buffer[level + 1].end();) {
			if ((*it)->get_min() > right || (*it)->get_max() < left) {
				++it;
			} else {
				if (utils::rmfile(sstable_dir_path_ + "/level-" + std::to_string(level + 1) + "/" + (*it)->get_filename())) {
					printf("rm level-%u sstable[%s] doesn't exist!_________\n", level + 1, (*it)->get_filename().c_str());
				}
				sstable_collection.push_back(*it);
				it = sstable_buffer[level + 1].erase(it);
			}
		}
	}
	std::vector<SSTableTuple> tuple_collection;
	uint64_t latest_timestamp = merge_sort_sstable(sstable_collection, tuple_collection);
	std::vector <SSTable *> new_table_list;
	uint64_t MAX_TUPLE = (SSTABLE_MAX_SIZE - 32 - 8192) / sizeof(SSTableTuple);
	auto it = tuple_collection.begin();
	for (; it + MAX_TUPLE <= tuple_collection.end(); it += MAX_TUPLE) {
		std::vector<SSTableTuple> subset(it, it + MAX_TUPLE);
		new_table_list.push_back(new SSTable(latest_timestamp, subset));
	}
	if (it != tuple_collection.end()) {
		std::vector<SSTableTuple> subset(it, tuple_collection.end());
		new_table_list.push_back(new SSTable(latest_timestamp, subset));
	}
	if (sstable_buffer.size() > level + 1) {
		sstable_buffer[level + 1].insert(sstable_buffer[level + 1].end(), new_table_list.begin(), new_table_list.end());
	} else {
		sstable_buffer.push_back(new_table_list);
	}
	if (!utils::dirExists(sstable_dir_path_ + "/level-" + std::to_string(level + 1))) {
		utils::mkdir(sstable_dir_path_ + "/level-" + std::to_string(level + 1));
	}
	for (SSTable *sstable: new_table_list) {
		std::ofstream out;
		out.open(sstable_dir_path_ + "/level-" + std::to_string(level + 1) + "/" + sstable->get_filename(), std::ios::binary);
		sstable->write_sstable(out);
		out.close();
	}
	for (SSTable *sstable: sstable_collection) {
		delete sstable;
	}
}

uint32_t KVStore::level_max_sstable_num(uint32_t level) const {
	return 1 << (level + 1);
}

void KVStore::remove_deleted_tuple(std::vector<SSTableTuple> &tuple_collection) const {
	for (auto it = tuple_collection.begin(); it != tuple_collection.end(); ) {
		if ((*it).v_len == 0) {
			it = tuple_collection.erase(it);
		} else {
			++it;
		}
	}
}

uint64_t KVStore::merge_sort_sstable(std::vector<SSTable *> &sstable_collection, std::vector<SSTableTuple> &tuple_collection) {
	auto cmp = [](const SSTableTimestampTuple &a, const SSTableTimestampTuple &b) {
		return a.tuple.key > b.tuple.key || (a.tuple.key == b.tuple.key && a.timestamp > b.timestamp);
	};
	std::priority_queue<SSTableTimestampTuple, std::vector<SSTableTimestampTuple>, decltype(cmp)> tuple_queue(cmp);
	uint64_t latest = 0;
	for (SSTable *sstable: sstable_collection) {
		latest = std::max(latest, sstable->get_timestamp());
		std::vector<SSTableTuple> content = sstable->get_content();
		for (SSTableTuple tuple: content) {
			tuple_queue.emplace(tuple.key, tuple.offset, tuple.v_len, sstable->get_timestamp());
		}
	}
	while (!tuple_queue.empty()) {
		SSTableTimestampTuple item = tuple_queue.top();
		tuple_queue.pop();
		if (tuple_collection.empty()) {
			tuple_collection.push_back(item.tuple);
		} else {
			if (item.tuple.key == tuple_collection.back().key) {
				tuple_collection.back().offset = item.tuple.offset;
				tuple_collection.back().v_len = item.tuple.v_len;
			} else {
				tuple_collection.push_back(item.tuple);
			}
		}
	}
	return latest;
}

void KVStore::print_sstable_buffer() const {
	printf("### SSTable Buffer:\n");
	for (uint32_t i = 0; i < sstable_buffer.size(); ++i) {
		printf("[level-%u]:\n", i);
		for (SSTable *sstable: sstable_buffer[i]) {
			printf("%s \n", sstable->get_filename().c_str());
			sstable->print();
		}
		printf("\n");
	}
	printf("\n");
}

void KVStore::print_sstable_disk() const {
	printf("### SSTable Disk:\n");
	std::vector<std::string> directory_list;
	utils::scanDir(sstable_dir_path_, directory_list);
	for (std::string dir: directory_list) {
		printf("files in[%s]:\n", dir.c_str());
		std::vector<std::string> sstable_list;
		utils::scanDir(sstable_dir_path_ + "/" + dir, sstable_list);
		for (std::string sstable_file_name: sstable_list) {
			printf("%s ", sstable_file_name.c_str());
		}
		printf("\n");
	}
	printf("\n");
}

void KVStore::print_memtable() const {
	this->skip_list_->print_pair();
}