cmake_minimum_required(VERSION 3.0)
project(display-zero LANGUAGES CXX)

find_file(DIR_LIBAVCODEC "libavcodec/avcodec.h")
if ("${DIR_LIBAVCODEC}" STREQUAL "DIR_LIBAVCODEC-NOTFOUND")
    message(FATAL_ERROR "avcodec.h not found")
endif()

find_file(DIR_LIBFREEIMAGE "FreeImage.h")
if ("${DIR_LIBFREEIMAGE}" STREQUAL "DIR_LIBFREEIMAGE-NOTFOUND")
    message(FATAL_ERROR "FreeImage.h not found")
endif()

find_file(DIR_LIBPIGPIO "pigpio.h")

if (NOT "${DIR_LIBPIGPIO}" STREQUAL "DIR_LIBPIGPIO-NOTFOUND")
    add_library(libdisplay-zero source/libdisplay-zero.cpp)
    target_link_libraries(libdisplay-zero PUBLIC pigpio)
    set_target_properties(libdisplay-zero PROPERTIES OUTPUT_NAME "display-zero")
endif()

add_executable(display-zero source/display-zero.cpp)
target_link_libraries(display-zero PUBLIC avcodec freeimage)
if (TARGET libdisplay-zero)
    target_compile_definitions(display-zero PUBLIC LIBDISPLAY_ZERO)
    target_link_libraries(display-zero PUBLIC libdisplay-zero)
endif()
set_target_properties(display-zero PROPERTIES OUTPUT_NAME "display-zero")