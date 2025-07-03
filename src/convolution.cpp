#include <algorithm>
#include <ivmg/Image.hpp>
#include <print>
#include <vector>


void ivmg::convolve_scalar_worker(const Image& img, const Filter& filter, Image& out, size_t start_pxl, size_t end_pxl) {

    std::vector<float> pxl_tmp(img.nb_channels);

    std::println("Processing pixels {} to {}", start_pxl, end_pxl);

    for(size_t i = start_pxl*img.nb_channels; i < end_pxl*img.nb_channels; i += img.nb_channels) {
        int ix = (i % (img.w * img.nb_channels)) / img.nb_channels;
        int iy = (i / (img.w * img.nb_channels));

        std::ranges::fill(pxl_tmp, 0.0f);

        for(int k = 0; k < filter.ksize * filter.ksize; k++) {
            // auto [kx, ky] = GetCoordsInFlatArray(k, filter.ksize);
            int kx = k % filter.ksize - filter.radius;
            int ky = k / filter.ksize - filter.radius;
            // kx -= filter.radius;
            // ky -= filter.radius;

            const uint32_t kix = ix + kx;
            const uint32_t kiy = iy + ky;

            // Boundary check. Acts as 0 padding.
            if( (kix < 0) || (kiy < 0) || (kix >= img.w) || (kiy >= img.h) ) continue;

            for(size_t c = 0; c < img.nb_channels; c++) {
                auto iidx = (kiy * img.w + kix) * img.nb_channels + c;

                pxl_tmp[c] += img.data[iidx] * filter.kernel[k];
            }
        }

        for(size_t c = 0; c < img.nb_channels; c++) {
            out.data[i + c] = static_cast<uint8_t>(std::clamp(pxl_tmp[c], 0.0f, 255.0f));
        }
    }
}

#ifdef __AVX512F__

void ivmg::convolve_avx512_worker(const Image &img, const Filter &filter, Image &out, size_t start_pxl, size_t end_pxl) {
    std::println("Hello I'm a very fast AVX 512 convolution but I don't do shit currently");
}

#endif