#include "qoi.hpp"
#include "common.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>

using namespace ivmg;

void ivmg::EncodeQOI(const Image &img, const std::filesystem::path& outfile) {

    std::ofstream out_file(outfile, std::ios::binary);

    qoi_header hdr {
        .width = std::byteswap(img.width()),
        .height = std::byteswap(img.height()),
        .channels = 4,
        .colorspace = 1
    };

    std::vector<uint8_t> qoi_magic = magics.at(Formats::QOI);
    out_file.write(reinterpret_cast<char*>(qoi_magic.data()), qoi_magic.size());
    out_file.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    Color prev_pxl = {0, 0, 0, 255}, current_pxl;
    uint8_t run = 0;

    std::array<Color, 64> pxl_cache {};

    for(size_t i = 0; i < img.width() * img.height(); i += BYTE_PER_PIXEL) {
        current_pxl = { img.get_raw_handle()[i], img.get_raw_handle()[i+1], img.get_raw_handle()[i+2], img.get_raw_handle()[i+3] };

        if(current_pxl == prev_pxl) {
            if(++run == 62) {
                out_file << static_cast<u8>(QOI_OP_RUN | (run - 1));
                run = 0;
            }
        }
        else {
            if(run != 0) {
                out_file << static_cast<u8>(QOI_OP_RUN | (run - 1));
                run = 0;
            }

            int pxl_hash = QOI_PIXEL_HASH(current_pxl[0], current_pxl[1], current_pxl[2], current_pxl[3]);
            
            if(current_pxl == pxl_cache[pxl_hash]) {
                out_file << static_cast<u8>(QOI_OP_INDEX | pxl_hash);
            }
            else {
                pxl_cache[pxl_hash] = current_pxl;

                std::array<i8, 4> diff = QOI_COLOR_DIFF(current_pxl, prev_pxl);

                i8 vrg = diff[0] - diff[1];
                i8 vbg = diff[2] - diff[1];

                // Same alpha
                if(diff[3] == 0) {

                    // Diff
                    if(diff[0] >= -2 && diff[0] <= 1 &&
                       diff[1] >= -2 && diff[1] <= 1 &&
                       diff[2] >= -2 && diff[2] <= 1 
                    ) {
                        out_file << static_cast<u8>(QOI_OP_DIFF | (diff[0] + 2) << 4 | (diff[1] + 2) << 2 | diff[2] + 2);
                    }
                    // Luma
                    else if(diff[2] >= -32 && diff[2] <= 31 &&
                            vrg >= -8 && vrg <= 7 &&
                            vbg >= -8 && vbg <= 7
                    ) {
                        out_file << static_cast<u16>(QOI_OP_LUMA | (diff[2] + 32) << 8 | (vrg + 8) << 4 | vbg + 8); 
                    }
                    // RGB
                    else {
                        out_file << static_cast<u32>(QOI_OP_RGB << 24 | current_pxl[0] << 16 | current_pxl[1] << 8 | current_pxl[2]);
                    }
                }
                // Different alpha, need to encode it
                else {
                    out_file << static_cast<u8>(QOI_OP_RGBA);
                    out_file << static_cast<u32>(current_pxl[0] << 24 | current_pxl[1] << 16 | current_pxl[2] << 8 | current_pxl[3]);
                }

            }

        }

    }
}

constexpr std::array<i8, 4> ivmg::QOI_COLOR_DIFF(Color &c1, Color &c2) {
    return { static_cast<i8>(c1[0] - c2[0]), static_cast<i8>(c1[1] - c2[1]), static_cast<i8>(c1[2] - c2[2]), static_cast<i8>(c1[3] - c2[3]) };
}
