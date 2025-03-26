#include <ivmg/ivmg.hpp>
#include <fstream>
#include <print>
#include "include/utils.hpp"
#include "codecs/png.hpp"

using namespace ivmg;


Image ivmg::open(std::string imgpath) {
    std::ifstream file(imgpath, std::ios::binary | std::ios::ate);

    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);

    char magic_buffer[max_magic_length];


    if(!file.read(magic_buffer, max_magic_length)) {
        std::println("Error reading {} bytes from {}", max_magic_length, imgpath);
    }

    uint8_t* file_buffer;

    for(auto& [ext, mag] : magics) {
        if(size_t mlen = sizeof(mag.data()); std::memcmp(magic_buffer, mag.data(), mlen) == 0) {
            file_buffer = new uint8_t[len-mlen];
            file.seekg(mlen);
            file.read(reinterpret_cast<char*>(file_buffer), len-mlen);
            file.close();

            if(!avail_decoders.contains(ext)) {
                std::println("Ivmg doesn't supports decoding images of this type yet");
                break;
            }

            switch(ext) {
                case Formats::PNG: {
                    return DecodePNG(file_buffer, len-mlen);
                }
            }
        }
    }
};