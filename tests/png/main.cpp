#include "lodepng.h"
#include "../common.hpp"
#include <cassert>
#include <stdexcept>

enum class FilterStrategy {
    Zero = 0,
    Sub = 1,
    Up = 2,
    Average = 3,
    Paeth = 4,
    ALL = 5,
};

void encode_png(const std::string& path, const std::vector<unsigned char>& image, unsigned w, unsigned h, char** argv) {
    std::vector<unsigned char> png;
    lodepng::State state;

    auto strategy = static_cast<FilterStrategy>(std::stoi(argv[1]));

    switch (strategy) {
        case FilterStrategy::Zero:    state.encoder.filter_strategy = LFS_ZERO; break;
        case FilterStrategy::Sub:     state.encoder.filter_strategy = LFS_ONE; break;
        case FilterStrategy::Up:      state.encoder.filter_strategy = LFS_TWO; break;
        case FilterStrategy::Average: state.encoder.filter_strategy = LFS_THREE; break;
        case FilterStrategy::Paeth:   state.encoder.filter_strategy = LFS_FOUR; break;
        case FilterStrategy::ALL:  state.encoder.filter_strategy = LFS_ENTROPY; break;
    }

    unsigned error = lodepng::encode(png, image, w, h, state);
    if (error) throw std::runtime_error("PNG encoding failed");
    lodepng::save_file(png, path);
}

int main(int argc, char** argv) {
    assert(argc == 2);

    return run_tests(argv, encode_png);
}

