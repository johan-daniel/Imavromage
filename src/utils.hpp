#pragma once

#include "Logger.hpp"
#include <ivmg/Image.hpp>
#include <cstring>
#include <utility>

using namespace ivmg;

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


// inline std::pair<size_t, size_t> GetCoordsInFlatArray(size_t idx, size_t width) {
//     return { idx % width, idx / width };
// }