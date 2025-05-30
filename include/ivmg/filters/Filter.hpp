#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include <cstdint>

namespace ivmg {
namespace filt {

class Filter {
    public:
        uint16_t ksize;
        uint16_t radius;
        double* kernel;

        Filter(uint16_t ks): ksize(ks % 2 != 0 ? ks : ks + 1),
            radius(static_cast<uint16_t>(ksize / 2)),
            kernel(new double[ksize * ksize])
        {
        }

        ~Filter() {
            delete[] kernel;
        }
};

}
}

#endif