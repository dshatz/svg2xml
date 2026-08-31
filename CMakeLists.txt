cmake_minimum_required(VERSION 3.14)
project(svg2xml CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(NanoSVG REQUIRED)
find_package(tinyxml2 REQUIRED)

add_executable(svg2xml main.cpp)
target_link_libraries(svg2xml PRIVATE
    NanoSVG::nanosvg
    tinyxml2::tinyxml2
)

