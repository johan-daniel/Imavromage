#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <functional>

#include "codecs/png.hpp"
#include "codecs/qoi.hpp"

const uint8_t BYTE_PER_PIXEL = 4;  // 4 channels @ 8 bits

enum class Formats : uint8_t {
    PNG,
    QOI
};

const std::unordered_map<Formats, std::vector<uint8_t> > magics = {
    { Formats::PNG, { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
    { Formats::QOI, {0x71, 0x6F, 0x69,  0x66} }
};

const uint8_t max_magic_length = 8;

typedef std::function<Image(uint8_t*, size_t)> Decoder_fn;
const std::unordered_map<Formats, Decoder_fn> avail_decoders = {
    { Formats::PNG, DecodePNG }
};

typedef std::function<void(const Image&, std::string)> Encoder_fn;
const std::unordered_map<Formats, Encoder_fn> encoders = {
    { Formats::QOI, EncodeQOI}
};
