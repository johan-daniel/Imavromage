#include "common.hpp"
#include "ivmg/core/formats.hpp"
#include "ivmg/core/image.hpp"
#include "ivmg/ivmg.hpp"

#include <cassert>
#include <iostream>
#include <random>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>

std::pair<unsigned, unsigned> get_random_size() {
    std::mt19937 rng(time(NULL));
    std::uniform_int_distribution<unsigned> dist(1, MAX_DIM);

    return { dist(rng), dist(rng) };
}

std::vector<unsigned char> generate_random_image(unsigned w, unsigned h) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<unsigned char> image(w * h * 4);
    for (auto& px : image) {
        px = static_cast<unsigned char>(dist(rng));
    }
    return image;
}

std::string create_temp_dir() {
    char tmpl[] = "/tmp/ivmg_test_XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (!dir) throw std::runtime_error("Failed to create temp dir");
    return std::string(dir);
}

void decode(const std::string& input_path, const std::string& output_path) {
    ivmg::Image img = ivmg::open(input_path);
    ivmg::save(img, output_path);
}

std::vector<unsigned char> read_pam(const std::string& path, unsigned w, unsigned h) {
    std::ifstream file(path, std::ios::binary);
    assert(file);

    std::string line;
    while (std::getline(file, line) && line != "ENDHDR") {
        // Skip headers
    }

    std::vector<unsigned char> data(w * h * 4);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    return data;
}

bool compare_images(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) {
        std::cout << "Image sizes differ";
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            std::cout << "Image mismatch at byte " << i;
            return false;
        }
    }

    return true;
}


bool run_tests(char** argv, EncodeFunction encode) {
    std::filesystem::path tmp = create_temp_dir();
    std::filesystem::path encoded_path = tmp / "encoded";
    std::filesystem::path decoded_path = tmp / "decoded.pam";

    try {
        for(int iteration = 0; iteration < ITERATIONS; iteration++) {
            auto [w, h] = get_random_size();

            auto img = generate_random_image(w, h);
            encode(encoded_path, img, w, h, argv);
            decode(encoded_path, decoded_path);

            std::vector<unsigned char> decoded = read_pam(decoded_path, w, h);
            if(!compare_images(img, decoded)) {
                // Don't clean up if there's an image mismatch
                return 1;
            }
        }
    }
    catch (const std::exception &e) {
        // Make sure we clean up
        std::filesystem::remove_all(tmp);
        std::cout << e.what();
        return 1;
    }

    std::filesystem::remove_all(tmp);
    return 0;
}
