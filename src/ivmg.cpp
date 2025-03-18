#include <ivmg/Image.hpp>
#include <ivmg/ivmg.hpp>
#include <print>

using namespace ivmg;


Image ivmg::open(std::string imgpath){
    std::println("Opening {}", imgpath);

    return Image(1, 1);
};