cmake_minimum_required(VERSION 3.13)

# Project configuration
project(multi_channel_diagnostic_test_rig VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include directories
include_directories(include)

# Add subdirectories based on target platform
if(PICO_PLATFORM)
    add_subdirectory(platforms/pico_w)
elseif(STM32_PLATFORM)
    add_subdirectory(platforms/nucleo_f446re)
endif()

# Add common source files
file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp")

# Create library for common code
add_library(diagnostic_core ${COMMON_SOURCES})
target_include_directories(diagnostic_core PUBLIC include)

# Add tests if enabled
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
