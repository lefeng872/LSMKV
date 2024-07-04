#pragma once

#include "kvstore_api.h"
#include "../skiplist/skiplist.h"
#include "../sstable/sstable.h"
#include "../vlog/vlog.h"
#include <vector>

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:
	SkipList *skip_list_;
	std::vector<SSTable *> sstable_list_;
	VLog *v_log;

	uint64_t global_timestamp;  // the latest sstable timestamp

	/**
	 * @param skip_list
	 * @return the byte size of the coresponding sstable of this current skip_list
	*/
	uint32_t get_skip_list_bytes(SkipList *skip_list);

	/**
	 * @brief allocate and construct a new sstable according to skip_list
	 * @param skip_list
	 * @return point to the new allocated sstable
	*/
	void flush_skip_list(SkipList * skip_list);
public:
	KVStore(const std::string &dir, const std::string &vlog);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;

	void scan(uint64_t key1, uint64_t key2, std::list<std::pair<uint64_t, std::string>> &list) override;

	void gc(uint64_t chunk_size) override;
};
