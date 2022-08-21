cmake_minimum_required(VERSION 3.0)
project(display-zero LANGUAGES CXX)

add_executable(display-zero display-zero.cpp)
target_link_libraries(display-zero PUBLIC pigpio freeimage)
