#pragma once

#include <atomic>
#include <functional>


namespace ivmg {

class Image;
namespace filt { class Filter; }


using cpu_convolve_fn_t = std::function<void(const Image&, const filt::Filter&, Image&, size_t, size_t)>;

#ifdef IVMG_GPU
using gpu_convolve_fn_t = void(const Image&, const Filter&, Image&);
#endif

struct IvmgCtx {
    std::atomic_bool init = false;
    cpu_convolve_fn_t cpu_convolution;

    static IvmgCtx& get();
};

}