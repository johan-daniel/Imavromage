#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>
#include "filters/Filter.hpp"


inline std::pair<int32_t, int32_t> GetCoordsInFlatArray(size_t idx, size_t width) {
    return { idx % width, idx / width };
}


using namespace ivmg::filt;

namespace ivmg {


enum class ColorType : uint8_t {
    RGBA = 0,
    RGB  = 1,
    YUV  = 2
};


const std::unordered_map<ColorType, uint8_t> ChannelNb4Type {
    { ColorType::RGBA, 4 },
    { ColorType::RGB,  3 },
    { ColorType::YUV,  3 }
};

union Pixel {
    uint8_t r,g,b,a;
    uint8_t c[4];
};


class Image {


    public:
        uint32_t width;     // In pixels  
        uint32_t height;    // In pixels
        ColorType color_type;
        uint8_t* data;

        Image(const uint32_t w, const uint32_t h): width(w), height(h), color_type(ColorType::RGBA) {
            data = new uint8_t[w * h * ChannelNb4Type.at(color_type)];     // TODO: Support other color types
        }

        ~Image() { delete[] data; }



        Image operator|(const Filter& f) {
            Image out { width, height };
            uint8_t channel_nb = ChannelNb4Type.at(color_type);
            const uint32_t width_bytes = width * channel_nb;

            std::vector<uint16_t> pxl_tmp(channel_nb);

            // We iterate pixel over pixel
            for(size_t i = 0; i < width_bytes * height; i += channel_nb) {
                auto [x, y] = GetCoordsInFlatArray(i, width_bytes);
                x /= channel_nb;

                pxl_tmp = { 0, 0, 0, 0 };

                for(int k = 0; k < f.ksize * f.ksize; k++) {
                    auto [kx, ky] = GetCoordsInFlatArray(k, f.ksize);
                    kx -= f.radius;
                    ky -= f.radius;

                    if( (x + kx < 0) || (y + ky < 0) 
                      || (x + kx >= width) || (y + ky >= height) ) continue;

                    for(size_t c = 0; c < channel_nb; c++) {
                        auto iidx = ((y + ky) * width_bytes + x + kx) + c;
                        auto kidx = ky * f.ksize + kx;

                        auto imgdata = data[iidx];
                        auto kval = f.kernel[kidx];

                        pxl_tmp[c] += imgdata * kval;
                        if(c == 0 || c == 3) out.data[i + c] = 255;
                    }
                }
            }

            return out;
        }

};


}