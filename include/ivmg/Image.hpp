#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include "filters/Filter.hpp"
#include "ComputeContext.hpp"


using namespace ivmg::filt;

namespace ivmg {

class Image;

//======================================================
// COLOR TYPE HELPERS
//======================================================

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
// WORKER STUBS FOR CONVOLUTION
//======================================================
void convolve_scalar_worker(const Image& img, const Filter& filter, Image& out, size_t start_pxl, size_t end_pxl);

#ifdef __AVX512F__
void convolve_avx512_worker(const Image& img, const Filter& filter, Image& out, size_t start_pxl, size_t end_pxl);
#endif

//======================================================
// MAIN IMAGE CLASS
//======================================================

/**
In memory buffer of raw decoded image data
*/
class Image {

    private:
        std::vector<uint8_t> data;  // In row major. x is col, y is row
        uint32_t w;     // In pixels  
        uint32_t h;    // In pixels
        
        friend void convolve_scalar_worker(const Image& img, const Filter& filter, Image& out, size_t start_pxl, size_t end_pxl);

    public:
        ColorType color_type;
        uint8_t nb_channels;

        Image(const uint32_t w, const uint32_t h, ColorType ct = ColorType::RGBA): data(w * h * ChannelNb4Type.at(ct)), w(w), h(h), color_type(ct)
        {
            nb_channels = ChannelNb4Type.at(color_type);
        }
        
        // ACCESSORS
        constexpr uint8_t* get_raw_handle() { return data.data(); }
        constexpr const uint8_t* get_raw_handle() const { return data.data(); }
        constexpr uint32_t width() const { return w; }
        constexpr uint32_t height() const { return h; }
        constexpr size_t size_bytes() const { return data.size(); }
        constexpr size_t size_pixels() const { return data.size() / ChannelNb4Type.at(color_type); }      // Assuming RGBA for now


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


        Image operator|(const Filter& f) {

            Image out { w, h };
            const size_t num_threads = std::thread::hardware_concurrency();
            const size_t pixels_per_thread = (w * h) / num_threads;

            auto fn = IvmgCtx::get().cpu_convolution;
 
            {
                std::vector<std::jthread> threads(num_threads);

                for(int i = 0; i < num_threads; i++) {
                    size_t start = i * pixels_per_thread;
                    size_t end = (i == num_threads - 1) ? w * h : start + pixels_per_thread;

                    threads.emplace_back(convolve_scalar_worker, std::cref(*this), std::cref(f), std::ref(out), start, end);
                }
            }

            return out;

        }

    
};


}