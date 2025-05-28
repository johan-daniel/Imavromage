#include <ivmg/ivmg.hpp>
#include <cstring>
#include <fstream>
#include "Logger.hpp"
#include "common.hpp"

#include "codecs/codecs.hpp"
#include "ivmg/Formats.hpp"

using namespace ivmg;

LOG_LEVEL Logger::level = LOG_LEVEL::WARNING;

Image ivmg::open(std::string imgpath) {
    std::ifstream file(imgpath, std::ios::binary | std::ios::ate);

    size_t len = file.tellg();
    file.seekg(0, std::ios::beg);

    char magic_buffer[max_magic_length];


    if(!file.read(magic_buffer, max_magic_length)) {
        Logger::log(LOG_LEVEL::ERROR, "Error reading {} bytes from {}", max_magic_length, imgpath);
        exit(1);
    }

    uint8_t* file_buffer;
    for(auto& [ext, mag] : magics) {
        if(size_t mlen = sizeof(mag.data()); std::memcmp(magic_buffer, mag.data(), mlen) == 0) {
            file_buffer = new uint8_t[len-mlen];
            file.seekg(mlen);
            file.read(reinterpret_cast<char*>(file_buffer), len-mlen);
            file.close();

            if(!avail_decoders.contains(ext)) {
                Logger::log(LOG_LEVEL::ERROR, "Ivmg does not support this type of images yet");
                exit(1);
            }

            return avail_decoders.at(ext)(file_buffer, len-mlen);
        }
    }
    Logger::log(LOG_LEVEL::ERROR, "Unknown format");
    exit(1);
};



void ivmg::save(const Image &img, const std::filesystem::path &imgpath, Formats target) {
    if(encoders.contains(target))
        encoders.at(target)(img, imgpath);
    else
        Logger::log(LOG_LEVEL::ERROR, "I shit pant while trying to encode {} image. SoonTM trust", imgpath.extension().string());
}
