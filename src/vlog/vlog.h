#ifndef LSM_KV_VLog_H
#define LSM_KV_VLog_H

#include <string>

struct VLogEntry {
    const uint8_t magic = 0xff;
    uint16_t check_sum;
    uint64_t key;
    uint32_t v_len;
    std::string value;
};

class VLog {
private:
    std::string file_path_;
    uint64_t head_;
    uint64_t tail_;
};

#endif //LSM_KV_VLog_H