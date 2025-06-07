cmake_minimum_required(VERSION 3.13)

# Project configuration
project(multi_channel_diagnostic_test_rig VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include directories
include_directories(include)
include_directories(platforms/common)

# Detect platform automatically if not specified
if(NOT DEFINED TARGET_PLATFORM)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(TARGET_PLATFORM "pico_w")
        message(STATUS "Auto-detected Pico W platform (PICO_SDK_PATH found)")
    else()
        set(TARGET_PLATFORM "host")
        message(STATUS "Building for host platform (no specific embedded target)")
    endif()
endif()

message(STATUS "Target Platform: ${TARGET_PLATFORM}")

# Add common source files (excluding platform-specific main files)
file(GLOB_RECURSE COMMON_SOURCES 
    "src/core/*.cpp"
    "src/ui/*.cpp" 
    "src/system/*.cpp"
    "src/monitoring/*.cpp"
    "src/logging/*.cpp"
    "src/utils/*.cpp"
    "src/demo/*.cpp"
)

# Create library for common code
if(COMMON_SOURCES)
    add_library(diagnostic_core ${COMMON_SOURCES})
    target_include_directories(diagnostic_core PUBLIC 
        include
        platforms/common
    )
    
    # Set compiler features for the library
    target_compile_features(diagnostic_core PUBLIC cxx_std_17)
else()
    message(WARNING "No common source files found in src/")
endif()

# Platform-specific builds
if(TARGET_PLATFORM STREQUAL "pico_w")
    # Check if we have Pico SDK
    if(DEFINED ENV{PICO_SDK_PATH})
        message(STATUS "Building for Pico W")
        add_subdirectory(platforms/pico_w)
    else()
        message(FATAL_ERROR "Pico W selected but PICO_SDK_PATH not set")
    endif()
    
elseif(TARGET_PLATFORM STREQUAL "nucleo_f446re" OR TARGET_PLATFORM STREQUAL "stm32")
    message(STATUS "Building for STM32 Nucleo-F446RE")
    add_subdirectory(platforms/nucleo_f446re)
    
elseif(TARGET_PLATFORM STREQUAL "host")
    message(STATUS "Building for host platform (testing/simulation)")
    
    # Create a host-based executable for testing
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main_host.cpp")
        add_executable(diagnostic_rig_host src/main_host.cpp)
        if(TARGET diagnostic_core)
            target_link_libraries(diagnostic_rig_host diagnostic_core)
        endif()
    else()
        message(STATUS "No host main file found - only building core library")
    endif()
    
else()
    message(FATAL_ERROR "Unknown platform: ${TARGET_PLATFORM}")
endif()

# Add tests if enabled
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    message(STATUS "Building tests")
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(WARNING "BUILD_TESTS enabled but tests/CMakeLists.txt not found")
    endif()
endif()

# Print summary
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Target Platform: ${TARGET_PLATFORM}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(TARGET diagnostic_core)
    message(STATUS "Core Library: YES")
else()
    message(STATUS "Core Library: NO (no source files found)")
endif()
message(STATUS "Tests: ${BUILD_TESTS}")
message(STATUS "===================================")
message(STATUS "")