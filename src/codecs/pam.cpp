#include "pam.hpp"
#include "core/common.hpp"
#include <ivmg/core/image.hpp>
#include <fstream>

void ivmg::EncodePAM(const Image &img, const std::filesystem::path& outfile) {
    std::ofstream outppm(outfile, std::ios::binary);
    outppm << "P7\n"
        << "WIDTH " << img.width() << "\n"
        << "HEIGHT " << img.height() << "\n"
        << "DEPTH " << 4 << "\n"
        << "MAXVAL 255\n"
        << "TUPLTYPE RGB_ALPHA\n"
        << "ENDHDR" << std::endl;

    outppm.write(reinterpret_cast<const char*>(img.get_raw_handle()), img.height() * img.width() * BYTE_PER_PIXEL);
    outppm.close();
}
