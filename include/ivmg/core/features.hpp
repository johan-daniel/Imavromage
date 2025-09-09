#pragma once



namespace ivmg {


enum class SIMD_LEVEL { SCALAR, AVX2, AVX512 };

struct IvmgRuntime {
    static SIMD_LEVEL get_simd_level();
    static bool gpu_available(); 
};



}