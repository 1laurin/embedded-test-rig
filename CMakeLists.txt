cmake_minimum_required(VERSION 3.13)

# Project configuration
project(multi_channel_diagnostic_test_rig VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# IMPORTANT: This root CMakeLists.txt is for HOST builds only
# Platform-specific builds must be run from their platform directories

message(STATUS "Multi-Channel Diagnostic Test Rig - Root Build")
message(STATUS "Note: This builds a host simulation for testing core logic")
message(STATUS "")
message(STATUS "For platform-specific builds:")
message(STATUS "  Pico W: ./scripts/build/build_pico.sh")
message(STATUS "  STM32:  ./scripts/build/build_nucleo.sh (future)")
message(STATUS "")

# Include directories for host build
include_directories(include)
include_directories(platforms/common)
include_directories(src/core)
include_directories(src/ui)
include_directories(src/system)

# Add source files (only non-empty ones)
set(COMMON_SOURCES "")
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
foreach(SRC ${ALL_SOURCES})
    file(SIZE ${SRC} SRC_SIZE)
    if(SRC_SIZE GREATER 0)
        list(APPEND COMMON_SOURCES ${SRC})
    endif()
endforeach()

# Create library for common code
if(COMMON_SOURCES)
    add_library(diagnostic_core ${COMMON_SOURCES})
    target_include_directories(diagnostic_core PUBLIC 
        include
        platforms/common
        src/core
        src/ui
        src/system
        src/demo
        src/utils
    )
    target_compile_features(diagnostic_core PUBLIC cxx_std_17)
    target_compile_definitions(diagnostic_core PUBLIC HOST_BUILD=1)
    
    list(LENGTH COMMON_SOURCES SRC_COUNT)
    message(STATUS "Created diagnostic_core library with ${SRC_COUNT} source files")
endif()

# Host test executable
file(WRITE ${CMAKE_BINARY_DIR}/host_main.cpp "
#include <iostream>
#include <cstdio>

// Stub implementations for host testing
extern \"C\" {
    void display_system_info() {
        printf(\"Host Build - Multi-Channel Diagnostic Test Rig\\n\");
        printf(\"This is a host simulation for testing core logic.\\n\");
        printf(\"For actual hardware, use platform-specific builds.\\n\");
    }
}

int main() {
    std::cout << \"Multi-Channel Diagnostic Test Rig - Host Build\" << std::endl;
    display_system_info();
    std::cout << \"Host build successful! Core library compiled.\" << std::endl;
    return 0;
}
")

add_executable(diagnostic_rig_host ${CMAKE_BINARY_DIR}/host_main.cpp)
if(TARGET diagnostic_core)
    target_link_libraries(diagnostic_rig_host diagnostic_core)
endif()

message(STATUS "Host build target: diagnostic_rig_host")
message(STATUS "Run with: make && ./diagnostic_rig_host")
