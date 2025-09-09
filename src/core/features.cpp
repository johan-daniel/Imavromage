#include <ivmg/core/features.hpp>

namespace ivmg {


#define runtime_supports_avx512 \
    __builtin_cpu_supports("avx512f") && \
    __builtin_cpu_supports("avx512bw") && \
    __builtin_cpu_supports("avx512cd") && \
    __builtin_cpu_supports("avx512dq")

#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__)
    #define compile_supports_avx512 1
#else
    #define compile_supports_avx512 0
#endif


SIMD_LEVEL IvmgRuntime::get_simd_level() {

    static SIMD_LEVEL level = [] () {
                    
        __builtin_cpu_init();

        #if compile_supports_avx512
            if(runtime_supports_avx512) return SIMD_LEVEL::AVX512;
        #endif

        #ifdef __AVX2__
            if(__builtin_cpu_supports("avx2")) return SIMD_LEVEL::AVX2;
        #endif

        return SIMD_LEVEL::SCALAR;
    }();

    return level;
}

bool IvmgRuntime::gpu_available() {
    return false;   
}

}