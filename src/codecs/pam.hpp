#pragma once

#include <filesystem>

namespace ivmg {

class Image;

void EncodePAM(const Image& img, const std::filesystem::path& outfile);


}
