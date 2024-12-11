#include "sstable.h"

SSTable::SSTable(uint64_t _timestamp, uint64_t _offset, std::vector<std::pair<uint64_t, std::string>> &content) {
	header_.timestamp = _timestamp;
    header_.size = content.size();
    header_.min = content.front().first;
    header_.max = content.back().first;
    filter_.reset();
    for (const auto &pair : content) {
        filter_.insert(pair.first);
        tuple_list_.push_back(SSTableTuple(pair.first, _offset, pair.second.length()));
		_offset += pair.second.length();
    }
}
