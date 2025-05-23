#include <print>
#include <ivmg/ivmg.hpp>
#include <ivmg/Image.hpp>
#include <ivmg/Formats.hpp>



int main(int argc, char** argv) {

    if(argc < 2) {
        std::println("Usage error : Not enough arguments!");
        return 1;
    }


    const std::string filepath(argv[1]);
    ivmg::Image img = ivmg::open(filepath);
    ivmg::save(img, "../resources/out.pam");


    return 0;

}