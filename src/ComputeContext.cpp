#include "../include/ivmg/ComputeContext.hpp"
#include "ivmg/Image.hpp"



ivmg::IvmgCtx& ivmg::IvmgCtx::get() {

    static IvmgCtx ctx;
    if (ctx.init) return ctx;

    ctx.cpu_convolution = ivmg::convolve_scalar_worker;

    __builtin_cpu_init();

    #ifdef __AVX512F__
    if(__builtin_cpu_supports("avx512f"))
        // ctx.cpu_convolution = ivmg::convolve_avx512_worker;
    #endif

    ctx.init = true;

    return ctx;
}
