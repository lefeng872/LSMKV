#include "vlog.h"
#include <fstream>

VLog::VLog(const std::string &_filename) {
    this->filename_ = _filename;
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
    in.seekg(this->tail_, std::ios::beg);
    VLogEntry entry;
    while (read_vlog_entry(in, &entry)) {
        // printf("this entry: key=%lu, vlen=%u, value=%s\n", entry.key, entry.v_len, entry.value.c_str());
        if (entry.check()) {
            printf("find the first entry\n");
            in.close();
            return;
        } else {
            printf("opps, not the right entry\n");
            this->tail_ += entry.size();
        }
    }
    printf("no entry found\n");
    in.close();
}

bool VLog::read_vlog_entry(std::ifstream &in, VLogEntry *vlog_entry) {
    unsigned char byte = 0;
    while (in.read(reinterpret_cast<char*>(&byte), 1)) {
        if (byte == 0xFF) {
            break;
        }
    }
    in.read(reinterpret_cast<char *> (&vlog_entry->check_sum), sizeof(vlog_entry->check_sum));
    in.read(reinterpret_cast<char *> (&vlog_entry->key), sizeof(vlog_entry->key));
    in.read(reinterpret_cast<char *> (&vlog_entry->v_len), sizeof(vlog_entry->v_len));
    char *value_str = new char[vlog_entry->v_len + 1];
    value_str[vlog_entry->v_len] = '\0';
    in.read(value_str, vlog_entry->v_len);
    vlog_entry->value = std::string(value_str);
    delete [] value_str;
    if (in.eof() || !in) {
        return false;
    }
    return true;
}

uint64_t VLog::append(const std::vector<std::pair<uint64_t, std::string>> &content) {
    std::ofstream out;
    out.open(this->filename_, std::ios::app | std::ios::binary);
    uint64_t offset = out.tellp();
    for (const auto &pair : content) {
        if (pair.second == "") {
            continue;
        }
        VLogEntry entry = VLogEntry(pair.first, pair.second);
        out.write(reinterpret_cast<const char *>(&entry.magic), sizeof(entry.magic));
        out.write(reinterpret_cast<const char *>(&entry.check_sum), sizeof(entry.check_sum));
        out.write(reinterpret_cast<const char *>(&entry.key), sizeof(entry.key));
        out.write(reinterpret_cast<const char *>(&entry.v_len), sizeof(entry.v_len));
        out.write(entry.value.c_str(), entry.value.size());
    }
    out.close();
    return offset;
}

void VLog::collect_garbage(uint64_t chunk_size, std::vector<GarbageEntry> &garbage) {
    uint64_t start = utils::seek_data_block(filename_);
    VLogEntry entry;
    std::ifstream in;
    in.open(filename_, std::ios::binary);
    while (tail_ - start < chunk_size) {
        if (read_vlog_entry(in, &entry)) {
            if (!entry.check()) {
                tail_ += entry.size();
                continue;
            }
            garbage.emplace_back(entry, tail_ + 1 + 2 + 8 + 4);
            tail_ += entry.size();
        } else {
            break;
        }
    }
    in.close();
    // printf("de_alloc(start=%lu, size=%lu)\n", start, chunk_size);
    utils::de_alloc_file(this->filename_, start, chunk_size);
}

std::string VLog::read_value(uint64_t offset, uint32_t len) {
    std::ifstream in;
    in.open(this->filename_, std::ios::binary);
    if (!in) {
        return "";
    }
    in.seekg(offset, std::ios::beg);
    std::string value = "";
    char *value_str = new char [len + 1];
    value_str[len] = '\0';
    in.read(value_str, len);
    value = std::string(value_str);
    delete [] value_str;
    in.close();
    return value;
}

void VLog::reset() {
    tail_ = 0;
    utils::rmfile(this->filename_);
    std::ofstream out;
    out.open(this->filename_, std::ios::binary);
    out.close();
}

void VLog::print() const {
    std::ifstream in;
    in.open(this->filename_, std::ios::binary);
    while (!in.eof() && in) {
        VLogEntry *vlog_entry = new VLogEntry();
        in.ignore(sizeof(vlog_entry->magic));
        in.read(reinterpret_cast<char *> (&vlog_entry->check_sum), sizeof(vlog_entry->check_sum));
        in.read(reinterpret_cast<char *> (&vlog_entry->key), sizeof(vlog_entry->key));
        in.read(reinterpret_cast<char *> (&vlog_entry->v_len), sizeof(vlog_entry->v_len));
        char *value_str = new char[vlog_entry->v_len + 1];
        value_str[vlog_entry->v_len] = '\0';
        in.read(value_str, vlog_entry->v_len);
        vlog_entry->value = std::string(value_str);
        delete [] value_str; 
        printf("magic=%hhu, check_sum=%hu, key=%lu, v_len=%u, value=%s\n", vlog_entry->magic, vlog_entry->check_sum, vlog_entry->key, vlog_entry->v_len, vlog_entry->value.c_str());
        delete vlog_entry;
    }
    in.close();
}