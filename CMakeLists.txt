cmake_minimum_required(VERSION 3.28)
project(jrt)

add_compile_options(-O3 -mavx2)

set(CMAKE_CXX_STANDARD 20)

add_executable(jrt main.cpp
        scalar.hpp)
