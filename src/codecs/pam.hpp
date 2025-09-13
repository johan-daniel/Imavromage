#pragma once

#include <filesystem>

namespace ivmg {

class Image;

void encode_pam(const Image& img, const std::filesystem::path& outfile);


}
