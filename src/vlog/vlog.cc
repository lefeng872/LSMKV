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
    this->tail_ = utils::seek_data_block(this->filename_);  // non-hole file region
    in.seekg(tail_);
    while (true) {
        VLogEntry entry;
        uint64_t result = read_vlog_entry(in, &entry);
        if (!result) {
            printf("no entry, empty vlog");
            in.close();
            return;
        }
        if (entry.check()) {
            break;
        }
        this->tail_ += entry.size();
    }
    in.close();
}

uint64_t VLog::read_vlog_entry(std::ifstream &in, VLogEntry *vlog_entry) {
    char byte = 0;
    while (byte != 0xff) {
        in.read(&byte, 1);
        if (in.eof()) {
            return 0;
        }
    }
    in.read(reinterpret_cast<char *> (&vlog_entry->check_sum), sizeof(vlog_entry->check_sum));
    in.read(reinterpret_cast<char *> (&vlog_entry->key), sizeof(vlog_entry->key));
    in.read(reinterpret_cast<char *> (&vlog_entry->v_len), sizeof(vlog_entry->v_len));
    uint64_t value_offset = in.tellg();
    char *value_str = new char[vlog_entry->v_len + 1];
    value_str[vlog_entry->v_len] = '\0';
    in.read(value_str, vlog_entry->v_len);
    vlog_entry->value = std::string(value_str);
    delete [] value_str;
    return value_offset;
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
        out.write(entry.value.c_str(), entry.v_len);
    }
    out.close();
    return offset;
}

std::string VLog::read_value(uint64_t offset, uint32_t len) {
    std::ifstream in;
    in.open(this->filename_, std::ios::binary);
    if (!in) {
        return "";
    }
    in.seekg(offset);
    std::string value = "";
    char *value_str = new char [len + 1];
    value_str[len] = '\0';
    in.read(value_str, len);
    value = std::string(value_str);
    delete [] value_str;
    in.close();
    return value;
}