#pragma once

#include "Image.hpp"
#include "ivmg/Formats.hpp"
#include <filesystem>

namespace ivmg {


Image open(const std::string& imgpath);
void save(const Image &img, const std::filesystem::path &imgpath, Formats target);


}
