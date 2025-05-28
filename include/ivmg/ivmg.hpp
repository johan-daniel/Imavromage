#pragma once

#include "Image.hpp"
#include <filesystem>

namespace ivmg {


Image open(std::string imgpath);
void save(const Image &img, const std::filesystem::path &imgpath);


}
