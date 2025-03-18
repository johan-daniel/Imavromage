#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unordered_map>


enum class Formats : uint8_t {
    PNG
};

const std::unordered_map<Formats, uint8_t*> magics = {
    { Formats::PNG, new uint8_t[]{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A} }
};

constexpr uint8_t max_magic_length = 8;




template <typename T, bool BE = false>
T Read(uint8_t* data, size_t& idx, size_t data_len) {
    T dest;

    size_t read_len = sizeof(T);
    if(size_t i = (idx + read_len); i > data_len)
        read_len = read_len - (i - data_len);
    
    std::memcpy(&dest, data+idx, read_len);
    idx += read_len;

    if constexpr (BE) {
        dest = std::byteswap(dest);
    }

    return dest;
}

inline uint8_t* ReadBytes(uint8_t* data, size_t& idx, size_t data_len, size_t N) {
    size_t read_len = N;
    if(size_t i = (idx + read_len); i > data_len)
        read_len = read_len - (i - data_len);
    
    uint8_t* dest = new uint8_t[read_len];
    std::memcpy(dest, data+idx, read_len);
    idx += read_len;

    return dest;
}




#endif