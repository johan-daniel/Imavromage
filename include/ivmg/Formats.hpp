#pragma once

#include <string>
#include <unordered_map>
namespace ivmg {

enum class Formats {
    PNG,
    QOI,
    PAM
};

const std::unordered_map<std::string, Formats> ext2format {
    { ".png", Formats::PNG },
    { ".qoi", Formats::QOI },
    { ".pam", Formats::PAM }
};

}