# Header
cmake_minimum_required(VERSION 3.18)
project(display-zero VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Depencencies
find_library(AVCODEC_DIR "avcodec")
if(NOT "${AVCODEC_DIR}" STREQUAL "AVCODEC_DIR-NOTFOUND")
    add_library(avcodec SHARED IMPORTED)
    set_property(TARGET avcodec PROPERTY IMPORTED_LOCATION "${AVCODEC_DIR}")
endif()

find_library(FREEIMAGE_DIR "freeimage")
if(NOT "${FREEIMAGE_DIR}" STREQUAL "FREEIMAGE_DIR-NOTFOUND")
    add_library(freeimage SHARED IMPORTED)
    set_property(TARGET freeimage PROPERTY IMPORTED_LOCATION "${FREEIMAGE_DIR}")
endif()

find_library(PIGPIO_DIR "pigpio")
if(NOT "${PIGPIO_DIR}" STREQUAL "PIGPIO_DIR-NOTFOUND")
    add_library(pigpio SHARED IMPORTED)
    set_property(TARGET pigpio PROPERTY IMPORTED_LOCATION "${PIGPIO_DIR}")
endif()

# Executable
set(PROJECT_TARGETS)
if(TARGET avcodec AND TARGET freeimage)
    add_executable(${CMAKE_PROJECT_NAME}-encode source/display-zero-encode.cpp)
    target_link_libraries(${CMAKE_PROJECT_NAME}-encode PRIVATE avcodec freeimage)
    set(PROJECT_TARGETS ${PROJECT_TARGETS} ${CMAKE_PROJECT_NAME}-encode)
else()
    message(WARNING "FreeImage and/or avcodec not found, display-zero-encode will not be compiled")
endif()

# Library
if(TARGET pigpio)
    add_library(${CMAKE_PROJECT_NAME} source/display-zero.cpp)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE pigpio)
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>")
    set(PROJECT_TARGETS ${PROJECT_TARGETS} ${CMAKE_PROJECT_NAME})

    add_executable(${CMAKE_PROJECT_NAME}-play source/display-zero-play.cpp)
    target_link_libraries(${CMAKE_PROJECT_NAME}-play PRIVATE ${CMAKE_PROJECT_NAME})
    set(PROJECT_TARGETS ${PROJECT_TARGETS} ${CMAKE_PROJECT_NAME}-play)
else()
    message(WARNING "PiGPIO not found, display-zero and display-zero-play will not be compiled")
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_SOURCE_DIR}/config.cmake.in"
    "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

write_basic_package_version_file("${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(TARGETS ${PROJECT_TARGETS}
    EXPORT ${CMAKE_PROJECT_NAME}_targets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT ${CMAKE_PROJECT_NAME}_targets
    FILE "${CMAKE_PROJECT_NAME}Targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

install(FILES "include/${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}")

install(FILES "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake" "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME}")

# Export
export(EXPORT ${CMAKE_PROJECT_NAME}_targets
    FILE "${CMAKE_PROJECT_NAME}Targets.cmake"
    NAMESPACE ${CMAKE_PROJECT_NAME}::)

# Uninstallation
add_custom_target(uninstall WORKING_DIRECTORY "${CMAKE_BUILD_DIR}" COMMAND cmake -P "${CMAKE_SOURCE_DIR}/uninstall.cmake" ${CMAKE_PROJECT_NAME})

# Documentation
find_package(Doxygen)
if (Doxygen_FOUND)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/documentation")
    add_custom_target(doc WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" COMMAND doxygen)
endif()