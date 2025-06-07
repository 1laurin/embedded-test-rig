cmake_minimum_required(VERSION 3.13)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory.")
endif()

# Project configuration
project(multi_channel_diagnostic_test_rig VERSION 1.0.0 LANGUAGES C CXX)

# Set standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection and configuration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect platform automatically if not specified
if(NOT DEFINED TARGET_PLATFORM)
    if(DEFINED ENV{PICO_SDK_PATH} AND EXISTS $ENV{PICO_SDK_PATH})
        set(TARGET_PLATFORM "pico_w")
        message(STATUS "Auto-detected Pico W platform (PICO_SDK_PATH found)")
    else()
        set(TARGET_PLATFORM "host")
        message(STATUS "Building for host platform (no specific embedded target)")
    endif()
endif()

message(STATUS "Target Platform: ${TARGET_PLATFORM}")

# Compiler options for host builds only
if(TARGET_PLATFORM STREQUAL "host")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    if(APPLE)
        # macOS specific flags
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    endif()
endif()

# Include directories
include_directories(include)
include_directories(platforms/common)

# Find common source files (only non-empty ones)
function(add_sources_from_dir DIR_PATH VAR_NAME)
    if(EXISTS ${DIR_PATH})
        file(GLOB DIR_SOURCES "${DIR_PATH}/*.cpp" "${DIR_PATH}/*.c")
        foreach(SRC ${DIR_SOURCES})
            file(SIZE ${SRC} SRC_SIZE)
            if(SRC_SIZE GREATER 0)
                list(APPEND ${VAR_NAME} ${SRC})
            endif()
        endforeach()
        set(${VAR_NAME} ${${VAR_NAME}} PARENT_SCOPE)
    endif()
endfunction()

# Collect common sources
set(COMMON_SOURCES "")
add_sources_from_dir("${CMAKE_CURRENT_SOURCE_DIR}/src/core" COMMON_SOURCES)
add_sources_from_dir("${CMAKE_CURRENT_SOURCE_DIR}/src/ui" COMMON_SOURCES)
add_sources_from_dir("${CMAKE_CURRENT_SOURCE_DIR}/src/system" COMMON_SOURCES)
add_sources_from_dir("${CMAKE_CURRENT_SOURCE_DIR}/src/demo" COMMON_SOURCES)
add_sources_from_dir("${CMAKE_CURRENT_SOURCE_DIR}/src/utils" COMMON_SOURCES)

# Create library for common code
if(COMMON_SOURCES)
    add_library(diagnostic_core ${COMMON_SOURCES})
    target_include_directories(diagnostic_core PUBLIC 
        include
        platforms/common
    )
    target_compile_features(diagnostic_core PUBLIC cxx_std_17)
    
    # Add platform-specific compile definitions
    if(TARGET_PLATFORM STREQUAL "pico_w")
        target_compile_definitions(diagnostic_core PUBLIC PLATFORM_PICO_W=1)
    elseif(TARGET_PLATFORM STREQUAL "host")
        target_compile_definitions(diagnostic_core PUBLIC PLATFORM_HOST=1)
    endif()
    
    message(STATUS "Common library created with ${list_length} source files")
else()
    message(WARNING "No common source files found in src/")
endif()

# Platform-specific builds
if(TARGET_PLATFORM STREQUAL "pico_w")
    # Check if we have Pico SDK
    if(DEFINED ENV{PICO_SDK_PATH} AND EXISTS $ENV{PICO_SDK_PATH})
        message(STATUS "Building for Pico W")
        add_subdirectory(platforms/pico_w)
    else()
        message(FATAL_ERROR "Pico W selected but PICO_SDK_PATH not set or invalid")
    endif()
    
elseif(TARGET_PLATFORM STREQUAL "nucleo_f446re" OR TARGET_PLATFORM STREQUAL "stm32")
    message(STATUS "Building for STM32 Nucleo-F446RE")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/platforms/nucleo_f446re/CMakeLists.txt")
        add_subdirectory(platforms/nucleo_f446re)
    else()
        message(WARNING "STM32 platform directory exists but no CMakeLists.txt found")
    endif()
    
elseif(TARGET_PLATFORM STREQUAL "host")
    message(STATUS "Building for host platform (testing/simulation)")
    
    # Create host executable using minimal files
    set(HOST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/host_main.cpp"
    )
    
    # Create minimal host main if it doesn't exist
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/host_main.cpp")
        message(STATUS "Creating minimal host main file")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/src/host_main.cpp"
"#include <iostream>
#include <cstdlib>

// Minimal host build for testing core library compilation

int main() {
    std::cout << \"Multi-Channel Diagnostic Test Rig - Host Build\" << std::endl;
    std::cout << \"This is a minimal host build for testing.\" << std::endl;
    std::cout << \"For full functionality, use Pico W or STM32 targets.\" << std::endl;
    return 0;
}
")
    endif()
    
    add_executable(diagnostic_rig_host "${CMAKE_CURRENT_SOURCE_DIR}/src/host_main.cpp")
    if(TARGET diagnostic_core)
        target_link_libraries(diagnostic_rig_host diagnostic_core)
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
list(LENGTH COMMON_SOURCES NUM_COMMON_SOURCES)
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Target Platform: ${TARGET_PLATFORM}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Common Sources: ${NUM_COMMON_SOURCES}")
if(TARGET diagnostic_core)
    message(STATUS "Core Library: YES")
else()
    message(STATUS "Core Library: NO")
endif()
message(STATUS "Tests: ${BUILD_TESTS}")
message(STATUS "===================================")
