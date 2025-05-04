#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <print>
#include <vector>
#include <unordered_map>
#include <ivmg/Image.hpp>
#include "codecs/png.hpp"

using namespace ivmg;

enum class Formats : uint8_t {
    PNG
};

const std::unordered_map<Formats, std::vector<uint8_t> > magics = {
    { Formats::PNG,  { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
};

uint8_t const max_magic_length = 8;

typedef std::function<Image(uint8_t*, size_t)> Decoder_fn;

const std::unordered_map<Formats, Decoder_fn> avail_decoders = {
    { Formats::PNG, DecodePNG }
};


template <typename T>
T Read(uint8_t* data, size_t& idx, size_t data_length) {
    T dest;

    size_t read_size = sizeof(T);
    std::println("Trying to read {} bytes @ {:#x}", read_size, reinterpret_cast<std::uintptr_t>(data+idx));

    if(idx + read_size > data_length) {
        read_size = read_size - ((idx + read_size) - (data_length));
    }

    std::memcpy(&dest, data+idx, read_size);
    idx += read_size;

    std::println("Read {} bytes @ {:#x}", read_size, reinterpret_cast<std::uintptr_t>(data+idx));

    return dest;
}
