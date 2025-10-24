#include "qoi.hpp"
#include "core/common.hpp"
#include <array>
#include <bit>
#include <filesystem>
#include <fstream>

using namespace ivmg;


void ivmg::encode_qoi(const Image &img, const std::filesystem::path &outfile) {
    std::println("Encoding in QOI");
    std::ofstream out(outfile, std::ios::out | std::ios::binary);

    qoi_header hdr {
        .width = std::byteswap(img.width()),
        .height = std::byteswap(img.height()),
        .channels = 4,
        .colorspace = QOI_COLORSPACE::SRGB
    };

    qoi_color_t prev_pxl = { 0, 0, 0, 255 };
    size_t run = 0;


    #define QOI_RUN \
        out.put(QOI_OP_RUN | (run - 1)); \
        run = 0;


    out.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));


    std::array<qoi_color_t, 64> color_cache {};


    for(size_t i = 0; i < img.size_bytes(); i += BYTE_PER_PIXEL) {
        qoi_color_t cur_pxl = { 
            img.get_raw_handle()[i], 
            img.get_raw_handle()[i + 1], 
            img.get_raw_handle()[i + 2], 
            img.get_raw_handle()[i + 3] 
        };

        if (cur_pxl == prev_pxl) {
            run++;
            if (run == 62 || run == img.size_bytes()) {
                QOI_RUN
            }
        }
        else {
            if (run > 0) {
                QOI_RUN
            }

            int pxl_hash = QOI_PIXEL_HASH(cur_pxl[0], cur_pxl[1], cur_pxl[2], cur_pxl[3]);

            if (color_cache.at(pxl_hash) == cur_pxl) {
                out.put(QOI_OP_INDEX | pxl_hash);
            }
            else {
                color_cache.at(pxl_hash) = cur_pxl;

                // Same alpha, we can diff
                if (cur_pxl[3] == prev_pxl[3]) {
                    qoi_diff_t diff = QOI_COLOR_DIFF(cur_pxl, prev_pxl);

                    i16 vg_r = diff[0] - diff[1];
                    i16 vg_b = diff[2] - diff[1];

                    if (
						diff[0] > -3 && diff[0] < 2 &&
						diff[1] > -3 && diff[1] < 2 &&
						diff[2] > -3 && diff[2] < 2
					) {
						out.put(QOI_OP_DIFF | (diff[0] + 2) << 4 | (diff[1] + 2) << 2 | (diff[2] + 2));
					}
                    else if (
						vg_r >  -9 && vg_r <  8 &&
						diff[2] > -33 && diff[2] < 32 &&
						vg_b >  -9 && vg_b <  8
					) {
						out.put(QOI_OP_LUMA | (diff[2] + 32));
						out.put(((vg_r + 8) << 4) | (vg_b +  8));
					}
                    else {
                        out << static_cast<u32>(QOI_OP_RGB | cur_pxl[0] << 16 | cur_pxl[1] << 8 | cur_pxl[2]);
					}
                }

                // Oh no
                else {
                    out.put(QOI_OP_RGBA);
                    out << static_cast<u32>(cur_pxl[0] << 24 | cur_pxl[1] << 16 | cur_pxl[2] << 8 | cur_pxl[3]);
                }
            }

        }

        prev_pxl = std::move(cur_pxl);
    }



    const char QOI_END_MARKER[8] = {0,0,0,0,0,0,0,1};
    out.write(QOI_END_MARKER, 8);

    #undef QOI_RUN
}

constexpr qoi_diff_t ivmg::QOI_COLOR_DIFF(qoi_color_t &c1, qoi_color_t &c2) {
    return { 
        static_cast<i16>(c1[0] - c2[0]), 
        static_cast<i16>(c1[1] - c2[1]), 
        static_cast<i16>(c1[2] - c2[2]), 
        static_cast<i16>(c1[3] - c2[3]) 
    };
}
