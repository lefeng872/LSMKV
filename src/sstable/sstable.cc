#include "sstable.h"

SSTable *SSTable::flush_skip_list(SkipList *skip_list, VLog *v_log) {
    std::vector<std::pair<uint64_t, std::string>> content;
	skip_list->get_content(content);
	uint64_t offset = v_log->append(content);
	this->header_.size = content.size();
	for (auto elem: content) {
		if (elem.first > this->header_.max) {
			this->header_.max = elem.first;
		}
		if (elem.first < this->header_.min) {
			this->header_.min = elem.first;
		}
		this->filter_.insert(elem.first);
        this->tuple_list_.push_back(SSTableTuple(elem.first, offset, elem.second.length()));
        offset += elem.second.length();
	}
}