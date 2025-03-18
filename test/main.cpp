#include <cstring>
#include <print>
#include <ivmg/ivmg.hpp>
#include <ivmg/Image.hpp>

// #include "Decoders.hpp"




int main(int argc, char** argv) {

    if(argc < 2) {
        std::println("Usage error : Not enough arguments!");
        return 1;
    }


    const std::string filepath(argv[1]);
    ivmg::Image img = ivmg::open(filepath);

    return 0;



    // std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    // size_t len = file.tellg();
    // file.seekg(0, std::ios::beg);
    
    // char magic_buffer[max_magic_length];
    

    // if(!file.read(magic_buffer, max_magic_length)) {
    //     std::println("Error reading {} bytes from {}", max_magic_length, filepath);
    //     return 1;
    // }

    // uint8_t* file_buffer;

    // for(auto& [ext, mag] : magics) {
    //     if(size_t mlen = sizeof(mag); std::memcmp(magic_buffer, mag, mlen) == 0) {
    //         file_buffer = new uint8_t[len-mlen];
    //         file.seekg(mlen);
    //         file.read(reinterpret_cast<char*>(file_buffer), len-mlen);
    //         file.close();

    //         switch(ext) {
    //             case Formats::PNG: {
    //                 DecodePNG(file_buffer, len-mlen);
    //                 break;
    //             }
    //         }
    //     }

    //     delete[] mag;
    // }

    // return 0;
}