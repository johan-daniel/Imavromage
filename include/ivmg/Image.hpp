#pragma once

#include <unordered_map>
#include <vector>
#include "filters/Filter.hpp"

using namespace ivmg::filt;

namespace ivmg {


enum class ColorType : uint8_t {
    RGBA,
    RGB,
    YCrCb
};

const std::unordered_map<ColorType, uint8_t> ChannelNb4Type {
    { ColorType::RGBA, 4 },
    { ColorType::RGB, 3 },
    { ColorType::YCrCb, 3 }
};


class Pixel {
private:
    ColorType type;
    std::vector<uint8_t> channels;
};


class Image {

    public:
        uint32_t width;
        uint32_t height;
        uint8_t* data;

        Image(uint32_t w, uint32_t h, uint8_t* d): width(w), height(h), data(d) {
        }

        ~Image() { delete[] data; }

        Image operator|(Filter f) {
            return Image(0, 0, {}); // Placeholder
        }

};


}