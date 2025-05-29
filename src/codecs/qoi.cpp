#include "qoi.hpp"
#include "common.hpp"
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>

using namespace ivmg;


void ivmg::EncodeQOI(const Image &img, const std::filesystem::path& outfile) {

    std::ofstream out_file(outfile, std::ios::binary);

    qoi_header hdr {
        .width = std::byteswap(img.width),
        .height = std::byteswap(img.height),
        .channels = 4,
        .colorspace = 1
    };

    std::vector<uint8_t> qoi_magic = magics.at(Formats::QOI);
    out_file.write(reinterpret_cast<char*>(qoi_magic.data()), qoi_magic.size());
    out_file.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    Color prev_pxl = {0, 0, 0, 255}, current_pxl;
    uint8_t run = 0;

    std::array<Color, 64> pxl_cache {};

    for(size_t i = 0; i < img.width * img.height; i += BYTE_PER_PIXEL) {
        current_pxl = { img.data[i], img.data[i+1], img.data[i+2], img.data[i+3] };

        if(current_pxl == prev_pxl && run++ < 62) continue;
        assert(run <= 62);
        out_file << static_cast<uint8_t>(QOI_OP_RUN | (run - 1));

        int pxl_hash = QOI_PIXEL_HASH(current_pxl[0], current_pxl[1], current_pxl[2], current_pxl[3]);
        if(current_pxl == pxl_cache[pxl_hash]) {
            out_file << static_cast<uint8_t>(QOI_OP_INDEX | pxl_hash);
            continue;
        }

        pxl_cache[pxl_hash] = current_pxl;
        // Color color_diff = QOI_COLOR_DIFF(static_cast<unsigned char>(reinterpret_cast<int*>(&prev_pxl)), current_pxl);
    }


}
