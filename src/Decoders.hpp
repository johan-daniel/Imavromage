#ifndef __DECODERS_HPP__
#define __DECODERS_HPP__

#include <cstddef>
#include <cstdint>
#include <vector>

enum class ChunkType : uint32_t { 
    IHDR = 0x49484452,
    PLTE = 0x504C5445,
    IDAT = 0x49444154,
    IEND = 0x49454E44,
};


struct ChunkPNG {
    uint32_t length;
    ChunkType type;
    uint32_t crc;
    uint8_t* data;
};



enum class PNG_COLOR_TYPE : uint8_t { 
    GSC = 0,
    RGB = 2,
    IDX = 3,
    GSCA = 4,
    RGBA = 6
};


struct PNG_IMG {
    uint32_t w, h;
    uint8_t bit_depth;
    PNG_COLOR_TYPE color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
    std::vector<uint8_t> compressed_data;
};


ChunkPNG ReadChunk(uint8_t* file_buffer, size_t &read_idx, size_t data_length);
void DecodePNG(uint8_t* file_buffer, size_t length);
void DecodeIHDR(uint8_t* data, uint32_t chunk_len, PNG_IMG& png);

#endif