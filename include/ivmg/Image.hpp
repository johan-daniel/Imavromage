#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <print>
#include <ranges>
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

        Image(const uint32_t w, const uint32_t h): width(w), height(h), color_type(ColorType::RGBA), 
            data(new uint8_t[w * h * ChannelNb4Type.at(color_type)]) 
        {
            std::fill_n(data, width * height * ChannelNb4Type.at(color_type), 0);
        }

        ~Image() { delete[] data; }



        Image operator|(const Filter& f) {
            Image out { width, height };
            uint8_t channel_nb = ChannelNb4Type.at(color_type);

            std::vector<double> pxl_tmp(channel_nb);

            // We iterate pixel over pixel
            for(size_t i = 0; i < width * height * channel_nb; i += channel_nb) {
                auto [ix, iy] = GetCoordsInFlatArray(i, width * channel_nb);
                ix /= channel_nb;

                std::ranges::fill(pxl_tmp, 0.0);

                for(int k = 0; k < f.ksize * f.ksize; k++) {
                    auto [kx, ky] = GetCoordsInFlatArray(k, f.ksize);
                    kx -= f.radius;
                    ky -= f.radius;

                    const uint32_t kix = ix + kx;
                    const uint32_t kiy = iy + ky;

                    // Boundary check. Acts as 0 padding.
                    if( (kix < 0) || (kiy < 0) || (kix >= width) || (kiy >= height) ) continue;

                    for(size_t c = 0; c < channel_nb; c++) {
                        auto iidx = (kiy * width + kix) * channel_nb + c;

                        pxl_tmp[c] += data[iidx] * f.kernel[k];
                    }
                }

                for(size_t c = 0; c < channel_nb; c++) {
                    out.data[i + c] = static_cast<uint8_t>(std::clamp(pxl_tmp[c], 0.0, 255.0));
                }
            }

            return out;
        }

};


}