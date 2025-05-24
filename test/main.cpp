#include "ivmg/formats.hpp"
#include <print>
#include <ivmg/ivmg.hpp>
#include <ivmg/Image.hpp>



int main(int argc, char** argv) {

    if(argc < 2) {
        std::println("Usage error : Not enough arguments!");
        return 1;
    }


    const std::string filepath(argv[1]);
    ivmg::Image img = ivmg::open(filepath);
    img.save("../resources/out.pam", ivmg::Formats::PNG);


    return 0;

}