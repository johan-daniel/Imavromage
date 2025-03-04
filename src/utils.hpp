#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>



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