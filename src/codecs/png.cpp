#include "Logger.hpp"
#include "utils.hpp"
#include "png.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <zlib.h>
#include <libdeflate.h>

using namespace ivmg;

bool PNG_Decoder::can_decode(std::ifstream& filestream) const {
    auto startpos = filestream.tellg();
    char buffer[magic_length] = {0};
    if(filestream.read(buffer, magic_length) && !filestream.fail() && std::memcmp(buffer, magic, magic_length) == 0) {
        filestream.seekg(startpos);
        return true;
    }
    return false;
}


Image PNG_Decoder::decode(std::ifstream& filestream) {
    filestream.seekg(0, std::ios_base::end);
    const size_t len = filestream.tellg();
    filestream.seekg(magic_length);

    Vec<u8> vbuffer;
    vbuffer.resize(len - magic_length);

    filestream.read(reinterpret_cast<char*>(vbuffer.data()), len - magic_length);

    return this->DecodePNG(vbuffer);
}


Image PNG_Decoder::DecodePNG(Vec<u8>& file_buffer) {
    // D(std::println("Decoding PNG");)
    Logger::log(LOG_LEVEL::INFO, "Decoding PNG of size {} bytes", file_buffer.size());
    auto start = std::chrono::high_resolution_clock::now();

    size_t pxl_idx {0};
    ChunkPNG chunk {};

    Vec<u8> rawidat;
    rawidat.reserve(file_buffer.size());

    do {
        chunk = this->ReadChunk(file_buffer, pxl_idx);

        Logger::log(LOG_LEVEL::INFO, "Got chunk {:#x} of length {} bytes", static_cast<u32>(chunk.type), chunk.length);

        switch(chunk.type) {

            case ChunkType::IHDR:
                this->DecodeIHDR(chunk.data);
                break;

            case ChunkType::PLTE:
                break;
            case ChunkType::IDAT: {
                rawidat.append_range(chunk.data);
                break;
            }
            case ChunkType::IEND:
                break;

        }
    } while(chunk.type != ChunkType::IEND);

    libdeflate_decompressor *decompressor = libdeflate_alloc_decompressor();

    libdeflate_result result = libdeflate_zlib_decompress(
        decompressor,
        rawidat.data(),
        rawidat.size(),
        inflated_data.data(),
        inflated_data.size(),
        nullptr
    );

    if(result != LIBDEFLATE_SUCCESS) {
        std::println(std::cerr, "Deflate died");
        exit(result);
    }

    libdeflate_free_decompressor(decompressor);

    // Reverse the filters
    Image img (width, height);

    const size_t scanline_size = width * bpp + 1;     // Width of the image + 1 byte for the filter type
    u8* scanline_buf = new u8[scanline_size];

    D(
        int n=0;
        int s=0;
        int u=0;
        int a=0;
        int p=0;
    )
    
    size_t write_idx = 0;
    pxl_idx = 0;

    while(pxl_idx < inflated_data.size()) {
        std::memcpy(scanline_buf, inflated_data.data() + pxl_idx, scanline_size);
        pxl_idx += scanline_size;
        PNG_FILT_TYPE filt = static_cast<PNG_FILT_TYPE>(scanline_buf[0]);

        switch(filt) {

        case PNG_FILT_TYPE::NONE: {
            D(n++;)
            
            std::vector<u8> linebuf(width * 4, 255);
            const u8 padding_nb = 4 - channel_nb.at(color_type);

            for(size_t i = 1; i < scanline_size-1; i += bpp) {
                const auto ipxl = (i / bpp) * (bpp + padding_nb);
                std::memcpy(linebuf.data() + ipxl, scanline_buf + i, bpp);
            }

            // Just copy everything
            std::memcpy(img.get_raw_handle() + write_idx, linebuf.data(), linebuf.size());
            write_idx += linebuf.size();

            break;
        }

        case PNG_FILT_TYPE::SUB: {
            D(s++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const u8 left = (sl_idx < bpp) ? 0 : img.get_raw_handle()[write_idx - bpp];    // Take the first pixel as is
                img.get_raw_handle()[write_idx++] = (left + inflated_data[pxl_idx + sl_idx + 1]) % 256;
            }

            break;
        }

        case PNG_FILT_TYPE::UP: {
            D(u++;)

            // Take the first line as is
            if(write_idx < scanline_size - 1) {
                std::memcpy(img.get_raw_handle() + write_idx, scanline_buf+1, scanline_size-1);
                write_idx += scanline_size-1;
                continue;
            }

            for(size_t sl_idx = 1; sl_idx < scanline_size; sl_idx++) {
                const u8 up = img.get_raw_handle()[write_idx - scanline_size + 1];
                img.get_raw_handle()[write_idx++] = (up + scanline_buf[sl_idx]) % 256;
            }

            break;
        }

        case PNG_FILT_TYPE::AVG: {
            D(a++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const uint16_t left = (sl_idx < bpp) ? 0 : img.get_raw_handle()[write_idx - bpp];    // Take the first pixel as is
                const uint16_t up = (write_idx < scanline_size - 1) ? 0 : img.get_raw_handle()[write_idx - scanline_size + 1];
                img.get_raw_handle()[write_idx++] = (scanline_buf[sl_idx + 1] + ((left + up) >> 1)) % 256;
            }

            break;
        }


        case PNG_FILT_TYPE::PAETH: {
            D(p++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const bool first_pixel = sl_idx < bpp;
                const bool first_scanline = write_idx < scanline_size - 1;

                const u8 left = first_pixel ? 0 : img.get_raw_handle()[write_idx - bpp];
                const u8 up = first_scanline ? 0 : img.get_raw_handle()[write_idx - scanline_size + 1];
                const u8 upleft = (first_pixel || first_scanline) ? 0 : img.get_raw_handle()[write_idx - scanline_size + 1 - bpp];

                img.get_raw_handle()[write_idx++] = (this->PaethPredictor(left, up, upleft) + scanline_buf[sl_idx + 1]) % 256;
            }

            break;
        }
        }
    }

    // delete[] inflated_data;
    delete[] scanline_buf;
    D(Logger::log(LOG_LEVEL::INFO, "Filter count - {} NONE - {} SUB - {} UP - {} AVG - {} PAETH", n,s,u,a,p);)
    auto end = std::chrono::high_resolution_clock::now();
    std::println("Decoded PNG of size {}x{} in {}", width, height, std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
    return img;
}


ChunkPNG PNG_Decoder::ReadChunk(Vec<u8>& data, size_t& idx) {
    ChunkPNG chunk {};
    chunk.length = Read<u32, BYTEORDER::BE>(data, idx);
    chunk.type = static_cast<ChunkType>(Read<u32, BYTEORDER::BE>(data, idx));
    chunk.data = std::span<u8>(data.begin() + idx, chunk.length);
    idx += chunk.length;
    chunk.crc = Read<u32, BYTEORDER::BE>(data, idx);

    return chunk;
}



void PNG_Decoder::DecodeIHDR(std::span<u8> data) {
    size_t idx {0};
    width = Read<u32, BYTEORDER::BE>(data, idx);
    height = Read<u32, BYTEORDER::BE>(data, idx);
    bit_depth = Read<u8, BYTEORDER::BE>(data, idx);
    color_type = static_cast<PNG_COLOR_TYPE>(Read<u8>(data, idx));
    compression_method = Read<u8>(data, idx);
    filter_method = Read<u8>(data, idx);
    interlace_method = Read<u8>(data, idx);
    bpp = channel_nb.at(color_type) * bit_depth / 8;
    inflated_data.resize(height * width * bpp + height);
}



i16 PNG_Decoder::PaethPredictor(u8 a, u8 b, u8 c) {
    const i16 p = a + b - c;
    const i16 pa = std::abs(p - a);
    const i16 pb = std::abs(p - b);
    const i16 pc = std::abs(p - c);

    if(pa <= pb && pa <= pc) return a;
    else if(pb <= pc) return b;
    else return c;
}
