#pragma once

#include <ivmg/core/image.hpp>
#include "utils.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>


namespace ivmg {

using namespace types;

typedef std::array<uint8_t, 4> Color;

const std::array<uint8_t, 8> QOI_END_MARKER = {0, 0, 0, 0, 0, 0, 0, 1};

// QOI chunks initialized. Tags are set but without data
const uint8_t QOI_OP_RGB   =  0xFE;     // 8 bits tag
const uint8_t QOI_OP_RGBA  =  0xFF;     // 8 bits tag
const uint8_t QOI_OP_LUMA  =  0x80;     // 2 bits tag
const uint8_t QOI_OP_INDEX =  0x00;     // 2 bits tag
const uint8_t QOI_OP_DIFF  =  0x40;     // 2 bits tag
const uint8_t QOI_OP_RUN   =  0xC0;     // 2 bits tag

#define QOI_PIXEL_HASH(r,g,b,a) (r * 3 + g * 5 + b * 7 + a * 11) % 64;

struct qoi_header {
    uint32_t width;             // image width in pixels (BE)
    uint32_t height;            // image height in pixels (BE)
    uint8_t  channels;          // 3 = RGB, 4 = RGBA
    uint8_t  colorspace;        // 0 = sRGB with linear alpha | 1 = all channels linear
} __attribute__((packed));


void EncodeQOI(const Image& img, const std::filesystem::path& outfile);

constexpr std::array<i8, 4> QOI_COLOR_DIFF(Color& c1, Color& c2);

}
