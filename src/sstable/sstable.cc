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
		_offset += pair.second.length() + 1 + 2 + 8 + 4;
    }
}

SSTable::SSTable(std::ifstream &in) {
	in.read(reinterpret_cast<char *> (&header_.timestamp), 8);
	in.read(reinterpret_cast<char *> (&header_.size), 8);
	in.read(reinterpret_cast<char *> (&header_.min), 8);
	in.read(reinterpret_cast<char *> (&header_.max), 8);
	in.read(reinterpret_cast<char *> (&filter_.bitset_), FILTER_MAX);
	SSTableTuple tuple;
	while (in.read(reinterpret_cast<char *> (&tuple), sizeof(tuple))) {
		tuple_list_.push_back(SSTableTuple(tuple.key, tuple.offset, tuple.v_len));
	}
}

SSTable::SSTable(uint64_t _timestamp, const std::vector<SSTableTuple> &content) {
	header_.timestamp = _timestamp;
	header_.size = content.size();
	header_.min = content.front().key;
	header_.max = content.back().key;
	for (SSTableTuple tuple: content) {
		this->filter_.insert(tuple.key);
		this->tuple_list_.push_back(tuple);
	}
}

std::string SSTable::get_filename() const {
	return std::to_string(header_.min) + "-" + std::to_string(header_.max) + "_" + std::to_string(header_.timestamp) + ".sst";
}

uint64_t SSTable::get_min() const {
	return this->header_.min;
}

uint64_t SSTable::get_max() const {
	return this->header_.max;
}

uint64_t SSTable::get_timestamp() const {
	return this->header_.timestamp;
}

void SSTable::write_sstable(std::ofstream &out) {
	out.write(reinterpret_cast<const char *> (&header_.timestamp), 8);
	out.write(reinterpret_cast<const char *> (&header_.size), 8);
	out.write(reinterpret_cast<const char *> (&header_.min), 8);
	out.write(reinterpret_cast<const char *> (&header_.max), 8);
	out.write(reinterpret_cast<const char *> (&filter_.bitset_), FILTER_MAX);
	for (SSTableTuple tuple: this->tuple_list_) {
		out.write(reinterpret_cast<const char *> (&tuple), sizeof(tuple));
	}
}

std::vector<SSTableTuple> SSTable::get_content() const {
	return tuple_list_;
}
