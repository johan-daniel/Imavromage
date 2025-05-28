#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <ivmg/Image.hpp>
#include <ivmg/Formats.hpp>
#include <unordered_map>

#include "png.hpp"
#include "qoi.hpp"
#include "pam.hpp"

namespace ivmg {

// Decoder functions registration
typedef std::function<Image(uint8_t*, size_t)> Decoder_fn;
const std::unordered_map<Formats, Decoder_fn> avail_decoders = {
    { ivmg::Formats::PNG, DecodePNG }
};

// Encoder functions registration
typedef std::function<void(const Image&, const std::filesystem::path&)> Encoder_fn;
const std::unordered_map<Formats, Encoder_fn> encoders = {
    { Formats::QOI, EncodeQOI },
    { Formats::PAM, EncodePAM }
};

}
