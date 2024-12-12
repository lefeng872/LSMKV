#ifndef LSM_KV_VLog_H
#define LSM_KV_VLog_H

#include <string>
#include <vector>
#include "../utils/utils.h"

struct VLogEntry {
    const uint8_t magic = 0xff;
    uint16_t check_sum;
    uint64_t key;
    uint32_t v_len;
    std::string value;

    VLogEntry() {
        check_sum = 0;
        key = 0;
        v_len = 0;
        value = "";
    }

    VLogEntry(uint64_t _key, std::string _value) {
        key = _key;
        v_len = _value.length();
        value = _value;
        std::vector<unsigned char> data;
        unsigned char *key_bytes = reinterpret_cast<unsigned char *>(&key);
        data.insert(data.end(), key_bytes, key_bytes + sizeof(key));
        unsigned char *vlen_bytes = reinterpret_cast<unsigned char *>(&v_len);
        data.insert(data.end(), vlen_bytes, vlen_bytes + sizeof(v_len));
        unsigned char *value_bytes = reinterpret_cast<unsigned char *>(&value);
        data.insert(data.end(), value_bytes, value_bytes + sizeof(value));
        check_sum = utils::crc16(data);
    }

    bool check() {
        std::vector<unsigned char> data;
        unsigned char *key_bytes = reinterpret_cast<unsigned char *>(&key);
        data.insert(data.end(), key_bytes, key_bytes + sizeof(key));
        unsigned char *vlen_bytes = reinterpret_cast<unsigned char *>(&v_len);
        data.insert(data.end(), vlen_bytes, vlen_bytes + sizeof(v_len));
        unsigned char *value_bytes = reinterpret_cast<unsigned char *>(&value);
        data.insert(data.end(), value_bytes, value_bytes + sizeof(value));
        return check_sum == utils::crc16(data);
    }

    uint64_t size() {
        return sizeof(magic) + sizeof(check_sum) + sizeof(key) + sizeof(v_len) + v_len;
    }
};

class VLog {
private:
    std::string filename_;
    uint64_t tail_;

    uint64_t read_vlog_entry(std::ifstream &in, VLogEntry *vlog_entry);
public:

    VLog(std::string _filename);
    /**
     * @brief append a list of key-value pairs to vLog file
     * @return offset of start
    */
    uint64_t append(const std::vector<std::pair<uint64_t, std::string>> &content);

    /**
     * @brief read a value with given offset and len
     * @param offset
     * @param len
     * @return value
     */
    std::string read_value(uint64_t offset, uint32_t len);
};

#endif //LSM_KV_VLog_H