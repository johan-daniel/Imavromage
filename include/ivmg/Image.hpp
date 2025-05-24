#pragma once

#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "formats.hpp"
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


        inline void save(std::string filepath, Formats target) {
            std::ofstream outppm(filepath, std::ios::binary);
            outppm << "P7\n"
                << "WIDTH " << width << "\n"
                << "HEIGHT " << height << "\n"
                << "DEPTH " << 4 << "\n"
                << "MAXVAL 255\n"
                << "TUPLTYPE RGB_ALPHA\n"
                << "ENDHDR" << std::endl;

            outppm.write(reinterpret_cast<char*>(data), height*width*4);
            outppm.close();
        }

};


}