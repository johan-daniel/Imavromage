#pragma once

// #include "core/image.hpp"
// #include "core/formats.hpp"
#include <filesystem>

namespace ivmg {

class Image;
enum class Formats;

Image open(const std::string& imgpath);
void save(const Image &img, const std::filesystem::path &imgpath, Formats target);


}
