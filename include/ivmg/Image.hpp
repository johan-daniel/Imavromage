#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include "filters/Filter.hpp"

using namespace ivmg::filt;

namespace ivmg {

class Image {

    public:
        uint32_t width;
        uint32_t height;
        uint8_t* data;

        Image(uint32_t w, uint32_t h): width(w), height(h) {
            data = new uint8_t[width * height * 4];
        }

        ~Image() {
            delete[] data;
        }

        Image operator|(Filter f) {
            return Image(0, 0); // Placeholder
        }

        Image operator>>(Filter f) {
            return *this | f;
        }

        inline void save(std::string filepath) {
            std::ofstream outppm(filepath, std::ios::binary);
            auto test = sizeof(data);
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