#pragma once

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

uint8_t const max_magic_length = 8;

const std::unordered_set<Formats> avail_decoders = {
    Formats::PNG
};


template <typename T>
T Read(uint8_t* data, size_t& idx) {
    T dest;

    std::memcpy(&dest, data+idx, sizeof(T));
    idx += sizeof(T);

    return dest;
}
