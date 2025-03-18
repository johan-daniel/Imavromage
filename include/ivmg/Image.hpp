#ifndef __IMAGE_HPP__
#define __IMAGE_HPP__

#include <cstdint>
#include "filters/Filter.hpp"

using namespace ivmg::filt;

namespace ivmg {

class Image {

    public:
        uint16_t width;
        uint16_t height;
        uint8_t* data;

        Image(uint16_t w, uint16_t h): width(w), height(h) {
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

};


}


#endif