#pragma once

#include <unordered_map>
#include "filters/Filter.hpp"

using namespace ivmg::filt;

namespace ivmg {


enum class ColorType : uint8_t {
    RGBA = 0,
    RGB  = 1,
    YUV  = 2
};


const std::unordered_map<ColorType, uint8_t> ChannelNb4Type {
    { ColorType::RGBA, 4 },
    { ColorType::RGB, 3 },
    { ColorType::YUV, 3 }
};


class Image {


    public:
        uint32_t width;
        uint32_t height;
        ColorType color_type;
        uint8_t* data;

        Image(const uint32_t w, const uint32_t h): width(w), height(h), color_type(ColorType::RGBA) {
            data = new uint8_t[w * h * ChannelNb4Type.at(color_type)];     // TODO: Support other color types
        }

        ~Image() { delete[] data; }



        Image operator|(const Filter& f) {
            Image out {width, height};
            uint8_t cnb = ChannelNb4Type.at(color_type);
            const auto width_bytes = width * cnb;

            for(size_t pxl = 0; pxl < width_bytes * height; pxl += cnb) {
                const int16_t x = pxl % width_bytes;
                const int16_t y = pxl / width_bytes;


                for(int xr = -f.radius; xr < f.radius; xr++) {
                    for(int yr = -f.radius; yr < f.radius; yr++) {
                        if( (x + xr < 0) || (y + yr < 0) ) continue;
                        for(size_t c = 0; c < cnb; c++) {
                            // out.data[i + c] += data[i + c + r * width * cnb] * f.kernel[r + r2 + 2 * f.radius];
                            auto imgdata = data[pxl + c + (xr * width_bytes) + yr] * f.kernel[yr * f.radius + xr];
                            if(c == 3) out.data[pxl + c] = 255;
                        }
                    }
                }
            }

            return out;
        }

};


}