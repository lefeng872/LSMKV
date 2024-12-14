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
        data.insert(data.end(), value.begin(), value.end());
        check_sum = utils::crc16(data);
        // printf("generate check_sum=%hu\n", check_sum); 
    }

    bool check() {
        std::vector<unsigned char> data;
        unsigned char *key_bytes = reinterpret_cast<unsigned char *>(&key);
        data.insert(data.end(), key_bytes, key_bytes + sizeof(key));
        unsigned char *vlen_bytes = reinterpret_cast<unsigned char *>(&v_len);
        data.insert(data.end(), vlen_bytes, vlen_bytes + sizeof(v_len));
        data.insert(data.end(), value.begin(), value.end());
        uint16_t result = utils::crc16(data);
        // printf("result=%hu, read check_sum=%hu\n", result, check_sum);
        return check_sum == result;
    }

    uint64_t size() {
        return sizeof(magic) + sizeof(check_sum) + sizeof(key) + sizeof(v_len) + v_len;
    }
};

class VLog {
private:
    std::string filename_;
    uint64_t tail_;

    bool read_vlog_entry(std::ifstream &in, VLogEntry *vlog_entry);
public:

    VLog(const std::string &_filename);
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

    void reset();

    void print() const;
};

#endif //LSM_KV_VLog_H