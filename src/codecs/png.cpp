#include "../include/utils.hpp"
#include "png.hpp"
#include <iostream>
#include <print>
#include <zlib.h>

Image DecodePNG(uint8_t* file_buffer, size_t length) {
    std::cout << "Decoding PNG..." << std::endl;

    PNG_IMG png {};
    size_t idx {0};

    bool eof = false;

    while(!eof) {
        ChunkPNG chunk = ReadChunk(file_buffer, idx, length);
        std::cout << "Got chunk " << std::hex << std::showbase << static_cast<uint32_t>(chunk.type)
            << std::dec << " of length " << chunk.length << std::endl;

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

    // png.compressed_data.shrink_to_fit();

    const size_t bpp = channel_nb.at(png.color_type) * png.bit_depth / 8;

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


    ivmg::Image img (png.w, png.h, dbuf);

    // Reverse the filters
    size_t scanline_size = png.w * bpp + 1;

    idx = 0;
    while(idx < dbuf_len) {
        uint8_t scanline_buf[scanline_size];
        ReadBytes(dbuf, scanline_buf, idx, dbuf_len, scanline_size);
        PNG_FILT_TYPE filt = static_cast<PNG_FILT_TYPE>(scanline_buf[0]);

        auto i = idx / scanline_size -1;

        switch(filt) {

        case PNG_FILT_TYPE::NONE:
            std::memcpy(img.data + idx - scanline_size - i, scanline_buf+1, scanline_size-1);
            break;

        case PNG_FILT_TYPE::SUB:
        case PNG_FILT_TYPE::UP:
        case PNG_FILT_TYPE::AVG:
        case PNG_FILT_TYPE::PAETH:
            std::println("Filter type {} is not yet supported", static_cast<uint8_t>(filt));
            break;

        }
    }

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