#pragma once

#include <filesystem>
#include <ivmg/Image.hpp>

namespace ivmg {

void EncodePAM(const Image& img, const std::filesystem::path& outfile);


}
