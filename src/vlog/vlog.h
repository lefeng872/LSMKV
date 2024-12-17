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

    VLogEntry(const VLogEntry& other) noexcept {
        check_sum = other.check_sum;
        key = other.key;
        v_len = other.v_len;
        value = other.value;
    }

    VLogEntry& operator=(VLogEntry&& other) noexcept {
        if (this != &other) {
            check_sum = other.check_sum;
            key = other.key;
            v_len = other.v_len;
            value = std::move(other.value);  // 使用 std::move 移动 value 字符串
        }
        return *this;
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

struct GarbageEntry {
    VLogEntry vlog_entry;
    uint64_t offset;
    GarbageEntry(VLogEntry _entry, uint64_t _offset) {
        vlog_entry.check_sum = _entry.check_sum;
        vlog_entry.key = _entry.key;
        vlog_entry.v_len = _entry.v_len;
        vlog_entry.value = _entry.value;
        offset = _offset;
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
     * collect the first n entries until recycled bytes >= chunk_size
     * then dig the hole only reclaim chunk_size space
     */
    void collect_garbage(uint64_t chunk_size, std::vector<GarbageEntry> &garbage);

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