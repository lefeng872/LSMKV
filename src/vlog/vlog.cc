#include "vlog.h"

VLog::VLog(std::string _filename) {
    this->filename_ = filename_;
    this->tail_ = 0;
}

uint64_t VLog::append(const std::vector<std::pair<uint64_t, std::string>> &content) {
    return 0;
}

std::string VLog::read_value(uint64_t offset, uint32_t len) {
    return "";
}