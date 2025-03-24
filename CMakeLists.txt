cmake_minimum_required(VERSION 3.25.1)

project("Imavromage" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HDRS "src/*.hpp")
file(GLOB_RECURSE SRCS "src/*.cpp")


find_package(ZLIB REQUIRED)

add_library(ivmg SHARED ${SRCS} ${HDRS})

set_target_properties(ivmg PROPERTIES
    VERSION ${PROJECT_VERSION}
    VISIBILITY_INLINES_HIDDEN TRUE
)

target_link_libraries(ivmg ZLIB::ZLIB)

target_include_directories(ivmg
    PUBLIC "./include/"
    PRIVATE "./src/"
)

include(GNUInstallDirs)

# Install public headers
install(DIRECTORY "./include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install shared library object
install(
    TARGETS ivmg
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


# Test executable for testing the API
file(GLOB_RECURSE TEST_SRCS "./test/*.cpp")
add_executable(ivmg_test ${TEST_SRCS})
target_link_libraries(ivmg_test ivmg)