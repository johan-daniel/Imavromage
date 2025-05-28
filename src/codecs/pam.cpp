#include "pam.hpp"
#include "common.hpp"
#include <fstream>

void ivmg::EncodePAM(const Image &img, const std::filesystem::path& outfile) {
    std::ofstream outppm(outfile, std::ios::binary);
    outppm << "P7\n"
        << "WIDTH " << img.width << "\n"
        << "HEIGHT " << img.height << "\n"
        << "DEPTH " << 4 << "\n"
        << "MAXVAL 255\n"
        << "TUPLTYPE RGB_ALPHA\n"
        << "ENDHDR" << std::endl;

    outppm.write(reinterpret_cast<char*>(img.data), img.height * img.width * BYTE_PER_PIXEL);
    outppm.close();
}
