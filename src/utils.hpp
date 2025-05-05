#pragma once

#include "Logger.hpp"
#include "codecs/png.hpp"
#include <cstring>
#include <functional>
#include <print>
#include <ivmg/Image.hpp>

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

    size_t read_size_attempt = sizeof(T);
    size_t read_size_actual = read_size_attempt;

    if(idx + read_size_attempt > data_length) 
        read_size_actual = read_size_attempt - ((idx + read_size_attempt) - data_length);

    std::memcpy(&dest, data+idx, read_size_actual);
    idx += read_size_actual;

    LOG_LEVEL lvl = (read_size_attempt == read_size_actual) ? LOG_LEVEL::INFO : LOG_LEVEL::WARNING;
    Logger::log(lvl, "Attempted to read {} bytes @ {:#x}, did read {}", read_size_attempt, reinterpret_cast<std::uintptr_t>(data+idx), read_size_actual);
    return dest;
}
