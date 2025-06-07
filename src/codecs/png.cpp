#include "ivmg/Image.hpp"
#include "utils.hpp"
#include "Logger.hpp"
#include "png.hpp"
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <zlib.h>

Image ivmg::DecodePNG(uint8_t* file_buffer, size_t length) {
    // D(std::println("Decoding PNG");)
    Logger::log(LOG_LEVEL::INFO, "Decoding PNG of size {} bytes", length);
    auto start = std::chrono::high_resolution_clock::now();

    PNG_IMG png {};
    size_t pxl_idx {0};

    ChunkPNG chunk {};

    do {
        chunk = ReadChunk(file_buffer, pxl_idx, length);
        // D(
        //     std::println("Got chunk {:#x} of length {}", static_cast<uint32_t>(chunk.type), chunk.length);
        // )


        Logger::log(LOG_LEVEL::INFO, "Got chunk {:#x} of length {} bytes", static_cast<uint32_t>(chunk.type), chunk.length);


        switch(chunk.type) {

            case ChunkType::IHDR:
                DecodeIHDR(chunk.data, chunk.length, png);
                break;

            case ChunkType::PLTE:
                break;
            case ChunkType::IDAT:
                png.compressed_data.insert(png.compressed_data.end(), chunk.data, chunk.data+chunk.length);
                break;
            case ChunkType::IEND:
                break;

        }
    } while(chunk.type != ChunkType::IEND);


    delete[] file_buffer;

    const size_t bpp = channel_nb.at(png.color_type) * png.bit_depth / 8;   // Bytes per pixel

    // Decompress the PNG data using zlib
    // TODO: Implement the Inflate algorithm myself
    size_t dbuf_len = png.w * png.h * bpp + png.h;
    uint8_t* dbuf = new uint8_t[dbuf_len];

    z_stream zstr {};
    zstr.next_out = dbuf;
    zstr.avail_out = dbuf_len;
    zstr.next_in = png.compressed_data.data();
    zstr.avail_in = png.compressed_data.size();

    int ret;
    ret = inflateInit(&zstr);

    if(ret != Z_OK) {
        Logger::log(LOG_LEVEL::ERROR, "Error initializing zlib stream: {}", ret);
        exit(ret);
    }

    while(ret != Z_STREAM_END) {
        ret = inflate(&zstr, Z_NO_FLUSH);

        if(ret != Z_OK && ret != Z_STREAM_END) {
            Logger::log(LOG_LEVEL::ERROR, "Inflate error encountered : {}", ret);
            exit(ret);
        }
    }

    ret = inflateEnd(&zstr);
    if(ret != Z_OK) {
        Logger::log(LOG_LEVEL::ERROR, "I shit pant while finishing inflating: {}", ret);
        exit(ret);
    }



    // Reverse the filters
    // uint8_t* outbuf = new uint8_t[png.w * png.h * 4];   // ivmg images are RGBA
    Image img (png.w, png.h);


    const size_t scanline_size = png.w * bpp + 1;     // Width of the image + 1 byte for the filter type
    uint8_t* scanline_buf = new uint8_t[scanline_size];


    pxl_idx = 0;

    D(
    int n=0;
    int s=0;
    int u=0;
    int a=0;
    int p=0;
    )

    size_t write_idx = 0;

    while(pxl_idx < dbuf_len) {
        std::memcpy(scanline_buf, dbuf + pxl_idx, scanline_size);
        pxl_idx += scanline_size;
        PNG_FILT_TYPE filt = static_cast<PNG_FILT_TYPE>(scanline_buf[0]);

        switch(filt) {

        case PNG_FILT_TYPE::NONE: {
            D(n++;)
            
            std::vector<uint8_t> linebuf(png.w * 4, 255);
            const uint8_t padding_nb = 4 - channel_nb.at(png.color_type);

            for(size_t i = 1; i < scanline_size-1; i += bpp) {
                const auto ipxl = (i / bpp) * (bpp + padding_nb);
                std::memcpy(linebuf.data() + ipxl, scanline_buf + i, bpp);
            }

            // Just copy everything
            std::memcpy(img.data + write_idx, linebuf.data(), linebuf.size());
            write_idx += linebuf.size();

            break;
        }

        case PNG_FILT_TYPE::SUB: {
            D(s++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const uint8_t left = (sl_idx < bpp) ? 0 : img.data[write_idx - bpp];    // Take the first pixel as is
                img.data[write_idx++] = (left + scanline_buf[sl_idx + 1]) % 256;
            }

            break;
        }

        case PNG_FILT_TYPE::UP: {
            D(u++;)

            // Take the first line as is
            if(write_idx < scanline_size - 1) {
                std::memcpy(img.data + write_idx, scanline_buf+1, scanline_size-1);
                write_idx += scanline_size-1;
                continue;
            }

            for(size_t sl_idx = 1; sl_idx < scanline_size; sl_idx++) {
                const uint8_t up = img.data[write_idx - scanline_size + 1];
                img.data[write_idx++] = (up + scanline_buf[sl_idx]) % 256;
            }

            break;
        }

        case PNG_FILT_TYPE::AVG: {
            D(a++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const uint16_t left = (sl_idx < bpp) ? 0 : img.data[write_idx - bpp];    // Take the first pixel as is
                const uint16_t up = (write_idx < scanline_size - 1) ? 0 : img.data[write_idx - scanline_size + 1];
                img.data[write_idx++] = (scanline_buf[sl_idx + 1] + ((left + up) >> 1)) % 256;
            }

            break;
        }


        case PNG_FILT_TYPE::PAETH: {
            D(p++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const bool first_pixel = sl_idx < bpp;
                const bool first_scanline = write_idx < scanline_size - 1;

                const uint8_t left = first_pixel ? 0 : img.data[write_idx - bpp];
                const uint8_t up = first_scanline ? 0 : img.data[write_idx - scanline_size + 1];
                const uint8_t upleft = (first_pixel || first_scanline) ? 0 : img.data[write_idx - scanline_size + 1 - bpp];

                img.data[write_idx++] = (PaethPredictor(left, up, upleft) + scanline_buf[sl_idx + 1]) % 256;
            }

            break;
        }
        }
    }

    delete[] dbuf;
    delete[] scanline_buf;
    D(Logger::log(LOG_LEVEL::INFO, "Filter count - {} NONE - {} SUB - {} UP - {} AVG - {} PAETH", n,s,u,a,p);)
    auto end = std::chrono::high_resolution_clock::now();
    std::println("Decoded PNG of size {}x{} in {}", png.w, png.h, std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
    return img;
}


ChunkPNG ivmg::ReadChunk(uint8_t* data, size_t& idx, size_t dlen) {
    ChunkPNG chunk {};
    chunk.length = std::byteswap(Read<uint32_t>(data, idx, dlen));
    chunk.type = static_cast<ChunkType>(std::byteswap(Read<uint32_t>(data, idx, dlen)));
    chunk.data = data + idx;
    idx += chunk.length;
    chunk.crc = std::byteswap(Read<uint32_t>(data, idx, dlen));

    return chunk;
}



void ivmg::DecodeIHDR(uint8_t *data, uint32_t chunk_len, PNG_IMG &png) {
    size_t idx {0};
    png.w = std::byteswap(Read<uint32_t>(data, idx, chunk_len));
    png.h = std::byteswap(Read<uint32_t>(data, idx, chunk_len));
    png.bit_depth = std::byteswap(Read<uint8_t>(data, idx, chunk_len));
    png.color_type = static_cast<PNG_COLOR_TYPE>(Read<uint8_t>(data, idx, chunk_len));
    png.compression_method = Read<uint8_t>(data, idx, chunk_len);
    png.filter_method = Read<uint8_t>(data, idx, chunk_len);
    png.interlace_method = Read<uint8_t>(data, idx, chunk_len);
}

int16_t ivmg::PaethPredictor(uint8_t a, uint8_t b, uint8_t c) {
    const int16_t p = a + b - c;
    const int16_t pa = std::abs(p - a);
    const int16_t pb = std::abs(p - b);
    const int16_t pc = std::abs(p - c);

    if(pa <= pb && pa <= pc) return a;
    else if(pb <= pc) return b;
    else return c;
}
