cmake_minimum_required(VERSION 3.25.1)

project("Imavromage")

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HDRS src/*.hpp)
file(GLOB_RECURSE SRCS src/*.cpp)

find_package(ZLIB REQUIRED)

add_executable(ivmg ${HDRS} ${SRCS})
target_link_libraries(ivmg ZLIB::ZLIB)