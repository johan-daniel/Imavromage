#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <unordered_set>


enum class Formats : uint8_t {
    PNG
};

const std::unordered_map<Formats, std::vector<uint8_t> > magics = {
    { Formats::PNG,  { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
};

constexpr uint8_t max_magic_length = 8;

const std::unordered_set<Formats> avail_decoders = {
    Formats::PNG
};



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

inline void ReadBytes(uint8_t* src, uint8_t* out, size_t& idx, size_t data_len, size_t N) {
    std::memcpy(out, src+idx, N);
    idx += N;
}