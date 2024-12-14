#pragma once

#include "kvstore_api.h"
#include "../skiplist/skiplist.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <vector>

static const int KB = 1024;

#define SSTABLE_MAX_SIZE (16 * 1024)

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
public:
	KVStore(const std::string &dir, const std::string &vlog);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;

	void gc(uint64_t chunk_size) override;

private:
	std::string sstable_dir_path_;
	std::string vlog_path_;
	SkipList *skip_list_;
	std::vector<std::vector<SSTable *>> sstable_buffer;
	VLog *v_log_;

	uint64_t global_timestamp;  // the next sstable timestamp to allocate

	/**
	 * SSTable must not larger than 16kb,
	 * must check does this memTable need to flush first
	 * before storing a new pair.
	*/
	uint32_t memTable_need_flush();

	/**
	 * @brief Put current skiplist into sstable
	 * in level-0, and continues the merging 
	 * process if neccessary.
	 * @details This function only put data onto
	 * disk, not modifying anything logically.
	*/
	uint32_t run_compaction();

	/**
	 * flush memtable to sstable on level-0
	 */
	void flush_memTable();

	/**
	 * 
	 */
	void merge_sstable_level0();
};
