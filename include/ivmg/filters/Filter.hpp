#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include <cstdint>

namespace ivmg {
namespace filt {

class Filter {
    public:
        const uint16_t ksize;
        const uint8_t* kernel;

        Filter(uint16_t ksize, uint8_t* k): ksize(ksize), kernel(k) {}

        virtual ~Filter() {
            delete[] kernel;
        }
};

}
}

#endif