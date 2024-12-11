#ifndef LSM_KV_VLog_H
#define LSM_KV_VLog_H

#include <string>
#include <vector>

struct VLogEntry {
    const uint8_t magic = 0xff;
    uint16_t check_sum;
    uint64_t key;
    uint32_t v_len;
    std::string value;
};

class VLog {
private:
    std::string filename_;
    uint64_t tail_;
public:

    VLog(std::string _filename);
    /**
     * @brief append a list of key-value pairs to vLog file
     * @return offset of start
    */
    uint64_t append(const std::vector<std::pair<uint64_t, std::string>> &content);

    std::string read_value(uint64_t offset, uint32_t len);
};

#endif //LSM_KV_VLog_H