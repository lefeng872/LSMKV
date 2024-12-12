#pragma once

#include "kvstore_api.h"
#include "../skiplist/skiplist.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include <stdio.h>
#include <vector>

static const std::string DEL = "~DELETED~";
static const int KB = 1024;

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
	SkipList *skip_list_;
	std::vector<std::vector<SSTable *>> sstable_list_;
	VLog *v_log_;

	uint64_t global_timestamp;  // the latest sstable timestamp

	/**
	 * SSTable must not larger than 16kb,
	 * must judge does this memTable need to flush first
	 * before storing a new pair.
	*/
	uint32_t memTable_need_flush();

	/**
	 * @brief run compaction for sstables
	 * @return the deepest level that is modified and need to be rewriten
	*/
	uint32_t run_compaction();

	/**
	 * @brief flush the modified sstables into disk
	 * @param level, the deepest level need to be flushed
	*/
	void flush_sstable(uint32_t level);
};
