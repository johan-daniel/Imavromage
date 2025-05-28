#include "argparse.hpp"
#include <ivmg/ivmg.hpp>
#include <ivmg/Image.hpp>
#include <ivmg/Formats.hpp>
#include <getopt.h>



int main(int argc, char** argv) {

    argparse::ArgumentParser program("ivmg-cli");
    program.add_argument("input")
        .help("specify the input file (or - for stdin)")
        .default_value("-");

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

    std::string input_file = program.get<std::string>("input");
    if(input_file == "-") 
        input_file = "/dev/stdin";

    std::string output_file = program.get<std::string>("--output");
    if(output_file == "-") 
        output_file = "/dev/stdout";

    ivmg::Image img = ivmg::open(input_file);
    ivmg::save(img, output_file, ivmg::Formats::PAM);


    return 0;

}
