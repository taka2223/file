cmake_minimum_required(VERSION 3.16)
project(filestream)

set(CMAKE_CXX_STANDARD 14)

add_executable(filestream main.cpp global.cpp global.h func.h func.cpp)