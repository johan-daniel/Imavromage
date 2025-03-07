#include "Decoders.hpp"
#include "utils.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <print>
#include <zlib.h>

void DecodePNG(uint8_t* file_buffer, size_t length) {
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
                png.compressed_data.insert(png.compressed_data.end(), &chunk.data[0], &chunk.data[chunk.length]);
                break;
            case ChunkType::IEND:
                eof = true;
                break;

        }

        delete[] chunk.data;
    }
    png.compressed_data.shrink_to_fit();


    // Decompress the PNG data using zlib
    // TODO: Implement the Inflate algorithm myself
    size_t dbuf_len = png.w * png.h * png.bit_depth;
    uint8_t* dbuf = (uint8_t*) malloc(dbuf_len);

    z_stream zstr {};
    zstr.next_out = dbuf;
    zstr.avail_out = dbuf_len;
    zstr.next_in = png.compressed_data.data();
    zstr.avail_in = png.compressed_data.size();

    int ret;
    ret = inflateInit(&zstr);

    if(ret != Z_OK) {
        std::print("Error initializing zlib stream: {}", ret);
        exit(ret);
    }
    
    while(ret != Z_STREAM_END) {
        ret = inflate(&zstr, Z_NO_FLUSH);

        if(ret != Z_OK && ret != Z_STREAM_END) {
            std::print("Inflate error encountered : {}", ret);
        }
    }

    // Reverse the filters

    delete[] file_buffer;
    delete[] dbuf;
}


ChunkPNG ReadChunk(uint8_t* data, size_t& idx, size_t dlen) {
    ChunkPNG chunk {};
    chunk.length = Read<uint32_t, true>(data, idx, dlen);
    chunk.type = static_cast<ChunkType>(Read<uint32_t, true>(data, idx, dlen));
    chunk.data = ReadBytes(data, idx, dlen, chunk.length);
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