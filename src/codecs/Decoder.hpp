#pragma once

#include "ivmg/Image.hpp"

namespace ivmg {

    
class Decoder {
public:
    virtual ~Decoder() = default;
    virtual bool can_decode(std::ifstream& filestream) const = 0;
    virtual Image decode(std::ifstream& filestream) = 0;
};



}