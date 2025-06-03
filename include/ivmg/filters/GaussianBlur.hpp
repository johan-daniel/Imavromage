#pragma once

#include "Filter.hpp"
#include <algorithm>
#include <cstddef>
#include "math.h"

namespace ivmg::filt {


class GaussianBlur: public Filter {

public:
    GaussianBlur(uint8_t ks): Filter(ks) {

        uint32_t sum = 0;

        // Compute the kernel values
        for(size_t i = 0; i < ksize * ksize; i++) {
            const int16_t x = i % ksize - radius;
            const int16_t y = i / ksize - radius;

            const double s = std::max(ksize / 2, 1);
            const double d = 2 * M_PI * (s * s);
            const double epwr = ((x*x) + (y*y)) / (2*(s*s));

            const double kval = std::exp(epwr) / d;;
            kernel[i] = kval;
            sum += kval;
        }

        // Normalise the kernel
        for(size_t i = 0; i < ksize * ksize; i++) {
            kernel[i] /= sum;
        }
    }


};





}