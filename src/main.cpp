#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <print>

#include "Decoders.hpp"

enum class Formats : uint8_t {
    PNG
};

const std::unordered_map<Formats, uint8_t*> magics = {
    { Formats::PNG, new uint8_t[]{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A} }
};

constexpr uint8_t max_magic_length = 8;

int main(int argc, char** argv) {

    if(argc < 2) {
        std::println("Usage error : Not enough arguments!");
        return 1;
    }


    const std::string filepath(argv[1]);
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);
    
    char magic_buffer[max_magic_length];
    

    if(!file.read(magic_buffer, max_magic_length)) {
        std::println("Error reading {} bytes from {}", max_magic_length, filepath);
        return 1;
    }

    uint8_t* file_buffer;

    for(auto& [ext, mag] : magics) {
        if(size_t mlen = sizeof(mag); std::memcmp(magic_buffer, mag, mlen) == 0) {
            file_buffer = new uint8_t[len-mlen];
            file.seekg(mlen);
            file.read(reinterpret_cast<char*>(file_buffer), len-mlen);
            file.close();

            switch(ext) {
                case Formats::PNG: {
                    DecodePNG(file_buffer, len-mlen);
                    break;
                }
            }
        }

        delete[] mag;
    }

    return 0;
}