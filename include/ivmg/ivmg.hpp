#pragma once

#include <filesystem>

namespace ivmg {

class Image;
enum class Formats;

Image open(const std::string& imgpath);
void save(const Image &img, const std::filesystem::path &imgpath, Formats target);


}
