#include "vlog.h"
#include <fstream>

VLog::VLog(std::string _filename) {
    this->filename_ = filename_;
    this->tail_ = 0;
    std::ifstream in;
    in.open(this->filename_, std::ios::binary);
    if (!in) {
        std::ofstream out;
        out.open(this->filename_, std::ios::binary);
        out.close();
        this->tail_ = 0;
        return;
    }
    // todo find tail
    this->tail_ = utils::seek_data_block(this->filename_);  // non-hole file region
    in.seekg(tail_);
    while (true) {
        VLogEntry entry;
        uint64_t result = read_vlog_entry(in, &entry);
        if (!result) {
            printf("Failed to read entry");
            return;
        }
        if (entry.check()) {
            break;
        }
        this->tail_ += entry.size();
    }
    in.close();
}

uint64_t VLog::append(const std::vector<std::pair<uint64_t, std::string>> &content) {
    std::ofstream out;
    out.open(this->filename_, std::ios::app | std::ios::binary);
    uint64_t offset = out.tellp();
    for (const auto &pair : content) {
        VLogEntry entry = VLogEntry(pair.first, pair.second);
        out.write(reinterpret_cast<const char *>(&entry.magic), sizeof(entry.magic));
        out.write(reinterpret_cast<const char *>(&entry.check_sum), sizeof(entry.check_sum));
        out.write(reinterpret_cast<const char *>(&entry.key), sizeof(entry.key));
        out.write(reinterpret_cast<const char *>(&entry.v_len), sizeof(entry.v_len));
        out.write(entry.value.c_str(), entry.value.length());
    }
    out.close();
    return offset;
}

std::string VLog::read_value(uint64_t offset, uint32_t len) {
    return "";
}