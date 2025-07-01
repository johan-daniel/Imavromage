#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include "filters/Filter.hpp"


inline std::pair<int32_t, int32_t> GetCoordsInFlatArray(size_t idx, size_t width) {
    return { idx % width, idx / width };
}


using namespace ivmg::filt;

namespace ivmg {


enum class ColorType : uint8_t {
    RGBA = 0,
    RGB  = 1,
    YUV  = 2
};


const std::unordered_map<ColorType, uint8_t> ChannelNb4Type {
    { ColorType::RGBA, 4 },
    { ColorType::RGB,  3 },
    { ColorType::YUV,  3 }
};


//======================================================
// PIXEL PROXIES TO MANIPULATE UNDERLYING DATA
//======================================================

struct PixelValue {
    uint8_t data[4];
    uint8_t nb_channels;

    PixelValue(const uint8_t* p, uint8_t nc): nb_channels(nc) {
        std::copy(p, p + nc, data);
        std::fill(data + nc, data + 4, 0);
    }
};

class PixelView {
    protected:
        uint8_t* ptr;
        uint8_t nb_channels;
        PixelView(uint8_t* p, uint8_t nc): ptr(p), nb_channels(nc) {}
    
    public:
        void operator+=(const PixelValue& other) {
            for(size_t i = 0; i < nb_channels; i++) {
                double buf = ptr[i] + other.data[i];
                ptr[i] = static_cast<uint8_t>(std::clamp(buf, 0.0, 255.0));
            }
        }

        PixelValue operator*(const double f) const {
            uint8_t buf[4];
            for(size_t i = 0; i < nb_channels; i++) {
                buf[i] = static_cast<uint8_t>(std::clamp(static_cast<double>(ptr[i]) * f, 0.0, 255.0));
            }
            return PixelValue(buf, 4);
        }

};

template <ColorType C>
struct Pixel : public PixelView {};

template <>
struct Pixel<ColorType::RGBA> : public PixelView {
    Pixel(uint8_t* p) : PixelView(p, 4) {}

    inline uint8_t& r() { return ptr[0]; }
    inline uint8_t& g() { return ptr[1]; }
    inline uint8_t& b() { return ptr[2]; }
    inline uint8_t& a() { return ptr[3]; }

    inline uint8_t r() const { return ptr[0]; }
    inline uint8_t g() const { return ptr[1]; }
    inline uint8_t b() const { return ptr[2]; }
    inline uint8_t a() const { return ptr[3]; }
};

template <>
struct Pixel<ColorType::RGB> : public PixelView {
    Pixel(uint8_t* p) : PixelView(p, 3) {}

    inline uint8_t& r() { return ptr[0]; }
    inline uint8_t& g() { return ptr[1]; }
    inline uint8_t& b() { return ptr[2]; }

    inline uint8_t r() const { return ptr[0]; }
    inline uint8_t g() const { return ptr[1]; }
    inline uint8_t b() const { return ptr[2]; }
};

template <>
struct Pixel<ColorType::YUV> : public PixelView {
    Pixel(uint8_t* p) : PixelView(p, 3) {}

    inline uint8_t& y() { return ptr[0]; }
    inline uint8_t& u() { return ptr[1]; }
    inline uint8_t& v() { return ptr[2]; }

    inline uint8_t y() const { return ptr[0]; }
    inline uint8_t u() const { return ptr[1]; }
    inline uint8_t v() const { return ptr[2]; }
};


using rgba_t = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;
using rgb_t = std::tuple<uint8_t, uint8_t, uint8_t>;
using yuv_t = std::tuple<uint8_t, uint8_t, uint8_t>;


//======================================================
// MAIN IMAGE CLASS
//======================================================

class Image {

    private:
        std::vector<uint8_t> data;  // In row major. x is col, y is row
        uint32_t w;     // In pixels  
        uint32_t h;    // In pixels
        
        public:
            ColorType color_type;
        // uint8_t* data;

        Image(const uint32_t w, const uint32_t h): data(w * h * 4, 255), w(w), h(h), color_type(ColorType::RGBA)
        {
        }

        // ACCESSORS
        constexpr uint8_t* get_raw_handle() { return data.data(); }
        constexpr const uint8_t* get_raw_handle() const { return data.data(); }
        constexpr uint32_t width() const { return w; }
        constexpr uint32_t height() const { return h; }
        constexpr size_t size_bytes() const { return data.size(); }
        constexpr size_t size_pixels() const { return data.size() / 4; }      // Assuming RGBA for now


        template <ColorType C>
        Pixel<C> at(size_t x, size_t y) {
            assert(x < w && y < h);

            size_t index = (y * w + x) * 4;
            return Pixel<C>(&data[index]);
        }

        template <ColorType C>
        const Pixel<C> at(size_t x, size_t y) const {
            assert(x < w && y < h);

            size_t index = (y * w + x) * 4;
            return Pixel<C>(&data[index]);
        }


        /**
        *   Convolution operator
        */
        Image operator|(const Filter& f) {
            Image out { w, h };
            uint8_t channel_nb = ChannelNb4Type.at(color_type);

            std::vector<double> pxl_tmp(channel_nb);

            // We iterate pixel over pixel
            for(size_t i = 0; i < w * h * channel_nb; i += channel_nb) {
                auto [ix, iy] = GetCoordsInFlatArray(i, w * channel_nb);
                ix /= channel_nb;

                std::ranges::fill(pxl_tmp, 0.0);

                for(int k = 0; k < f.ksize * f.ksize; k++) {
                    auto [kx, ky] = GetCoordsInFlatArray(k, f.ksize);
                    kx -= f.radius;
                    ky -= f.radius;

                    const uint32_t kix = ix + kx;
                    const uint32_t kiy = iy + ky;

                    // Boundary check. Acts as 0 padding.
                    if( (kix < 0) || (kiy < 0) || (kix >= w) || (kiy >= h) ) continue;

                    for(size_t c = 0; c < channel_nb; c++) {
                        auto iidx = (kiy * w + kix) * channel_nb + c;

                        pxl_tmp[c] += data[iidx] * f.kernel[k];
                    }
                }

                for(size_t c = 0; c < channel_nb; c++) {
                    out.data[i + c] = static_cast<uint8_t>(std::clamp(pxl_tmp[c], 0.0, 255.0));
                }
            }

            return out;
        }

        // Image operator|(const Filter& f) {
        //     Image out { w, h };

        //     for(size_t i = 0; i < w; i++) {
        //         for(size_t j = 0; j < h; j++) { 
        //             for(size_t k = 0; k < f.ksize * f.ksize; k++) {
        //                 const int kx = k % f.ksize - f.radius;
        //                 const int ky = k / f.ksize - f.radius;

        //                 const int kix = i - kx;
        //                 const int kjy = j - ky;

        //                 // Boundary check. Acts as 0 padding.
        //                 if( (kix < 0) || (kjy < 0) || (kix >= w) || (kjy >= h) ) continue;

        //                 out.at<ColorType::RGBA>(i,j) += this->at<ColorType::RGBA>(kix, kjy) * f.kernel[k];
        //             }
        //         }
        //     }

        //     return out;
        // }

};


}