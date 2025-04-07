#pragma once

#include <string>
#include <fstream>
#include "filters/Filter.hpp"

using namespace ivmg::filt;

namespace ivmg {


union Pixel {
    struct {
        uint8_t r, g, b, a;
    };
    uint8_t rgba[4];
};


class Image {

    public:
        uint32_t width;
        uint32_t height;
        uint8_t* data;

        Image(uint32_t w, uint32_t h, uint8_t* d): width(w), height(h), data(d) {
        }

        ~Image() { delete[] data; }

        // Image operator|(Filter f) {
        //     return Image(0, 0, {}); // Placeholder
        // }

        // Image operator>>(Filter f) {
        //     return *this | f;
        // }

        inline void save(std::string filepath) {
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