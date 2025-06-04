#include "argparse.hpp"
#include "ivmg/filters/GaussianBlur.hpp"
#include <ivmg/ivmg.hpp>
#include <ivmg/Image.hpp>
#include <ivmg/Formats.hpp>
#include <getopt.h>



int main(int argc, char** argv) {

    argparse::ArgumentParser program("ivmg-cli");
    program.add_argument("-i", "--input")
        .help("specify the input file")
        .required();

    program.add_argument("-o", "--output")
        .help("specify the output file (or - for stdout)")
        .default_value("out.pam");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input_file = program.get<std::string>("--input");

    std::string output_file = program.get<std::string>("--output");
    if(output_file == "-")
        output_file = "/dev/stdout";

    ivmg::Image img = ivmg::open(input_file);
    ivmg::Image img2 = img | GaussianBlur(7, 20);
    ivmg::save(img2, output_file, ivmg::Formats::PAM);


    return 0;

}
