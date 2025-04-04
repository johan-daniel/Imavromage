#include "../include/utils.hpp"
#include "png.hpp"
#include <iostream>
#include <print>
#include <zlib.h>

Image DecodePNG(uint8_t* file_buffer, size_t length) {
    D(std::cout << "Decoding PNG..." << std::endl;)

    PNG_IMG png {};
    size_t pxl_idx {0};

    bool eof = false;

    while(!eof) {
        ChunkPNG chunk = ReadChunk(file_buffer, pxl_idx, length);
        D(std::cout << "Got chunk " << std::hex << std::showbase << static_cast<uint32_t>(chunk.type)
            << std::dec << " of length " << chunk.length << std::endl;)

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
                eof = true;
                break;

        }
    }
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
        std::println("Error initializing zlib stream: {}", ret);
        exit(ret);
    }

    while(ret != Z_STREAM_END) {
        ret = inflate(&zstr, Z_NO_FLUSH);

        if(ret != Z_OK && ret != Z_STREAM_END) {
            std::println("Inflate error encountered : {}", ret);
        }
    }

    ret = inflateEnd(&zstr);
    if(ret != Z_OK) {
        std::println("I shit pant while finishing inflating: {}", ret);
        exit(ret);
    }



    // Reverse the filters
    uint8_t* outbuf = new uint8_t[png.w * png.h * 4];   // ivmg images are RGBA
    ivmg::Image img (png.w, png.h, outbuf);

    size_t scanline_size = png.w * bpp + 1;     // Width of the image + 1 byte for the filter type

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
        uint8_t scanline_buf[scanline_size];
        ReadBytes(dbuf, scanline_buf, pxl_idx, dbuf_len, scanline_size);
        PNG_FILT_TYPE filt = static_cast<PNG_FILT_TYPE>(scanline_buf[0]);

        switch(filt) {

        case PNG_FILT_TYPE::NONE: {
            D(n++;)

            // Just copy everything
            std::memcpy(img.data + write_idx, scanline_buf+1, scanline_size-1);
            write_idx += scanline_size-1;

            break;
        }

        case PNG_FILT_TYPE::SUB: {
            D(s++;)

            for(size_t sl_idx = 0; sl_idx < scanline_size-1; sl_idx++) {
                const uint8_t prev = (sl_idx < bpp) ? 0 : img.data[write_idx - bpp];    // Take the first pixel as is
                img.data[write_idx++] = (prev + scanline_buf[sl_idx + 1]) % 256;
            }

            break;
        }

        case PNG_FILT_TYPE::UP: {
            D(u++;)

            // Take the first line as is
            if(write_idx <= scanline_size) {
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
                const uint16_t prev = (sl_idx < bpp) ? 0 : img.data[write_idx - bpp];    // Take the first pixel as is
                const uint16_t up = (write_idx <= scanline_size) ? 0 : img.data[write_idx - scanline_size + 1];
                img.data[write_idx++] = (scanline_buf[sl_idx + 1] + ((prev + up) >> 1)) % 256;
            }

            break;
        }


        case PNG_FILT_TYPE::PAETH:
            p++;
            std::println("Filter type {} is not yet supported", static_cast<uint8_t>(filt));
            break;

        }
    }

    delete[] dbuf;
    D(std::println("Filter count - {} NONE - {} SUB - {} UP - {} AVG - {} PAETH", n,s,u,a,p);)

    return img;
}


ChunkPNG ReadChunk(uint8_t* data, size_t& idx, size_t dlen) {
    ChunkPNG chunk {};
    chunk.length = Read<uint32_t, true>(data, idx, dlen);
    chunk.type = static_cast<ChunkType>(Read<uint32_t, true>(data, idx, dlen));
    chunk.data = data + idx;
    idx += chunk.length;
    chunk.crc = Read<uint32_t, true>(data, idx, dlen);

    return chunk;
}



void DecodeIHDR(uint8_t *data, uint32_t chunk_len, PNG_IMG &png) {
    size_t idx {0};
    png.w = Read<uint32_t, true>(data, idx, chunk_len);
    png.h = Read<uint32_t, true>(data, idx, chunk_len);
    png.bit_depth = Read<uint8_t, true>(data, idx, chunk_len);
    png.color_type = static_cast<PNG_COLOR_TYPE>(Read<uint8_t>(data, idx, chunk_len));
    png.compression_method = Read<uint8_t>(data, idx, chunk_len);
    png.filter_method = Read<uint8_t>(data, idx, chunk_len);
    png.interlace_method = Read<uint8_t>(data, idx, chunk_len);

    return;
}