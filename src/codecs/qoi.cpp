#include "qoi.hpp"
#include "common.hpp"
#include <bit>
#include <cstdint>
#include <fstream>

using namespace ivmg;


void ivmg::EncodeQOI(const Image &img, std::string outfile) {

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

    for(size_t i = 0; i < hdr.width * hdr.height; i+=BYTE_PER_PIXEL) {
        current_pxl = { img.data[i], img.data[i+1], img.data[i+2], img.data[i+3] };

        if(current_pxl == prev_pxl && run++ < 62) continue;
        assert(run <= 62);
        out_file << static_cast<uint8_t>(QOI_OP_RUN | (run - 1));
        
        
    }


}