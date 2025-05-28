#pragma once

#include <functional>
#include <string>
#include <vector>

#define MAX_DIM 2048
#define ITERATIONS 5

typedef std::function<void(const std::string&, const std::vector<unsigned char>&, 
                           unsigned, unsigned, char**)> EncodeFunction;

bool run_tests(char** argv, EncodeFunction encode);
