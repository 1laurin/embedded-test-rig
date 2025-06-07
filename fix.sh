#!/bin/bash

# Complete Build System Setup and Fix Script
# This script sets up the proper build environment and fixes common issues

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Multi-Channel Diagnostic Test Rig Build Setup ===${NC}"
echo "Project root: $PROJECT_ROOT"
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect platform
detect_platform() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "linux"* ]]; then
        echo "linux"
    else
        echo "unknown"
    fi
}

# Check for required tools
echo -e "${YELLOW}Checking build requirements...${NC}"

# Check CMake
if command_exists cmake; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo -e "${GREEN}✓ CMake found: $CMAKE_VERSION${NC}"
else
    echo -e "${RED}✗ CMake not found${NC}"
    echo "Please install CMake 3.13 or later"
    exit 1
fi

# Check for Pico SDK
echo ""
echo -e "${YELLOW}Checking Pico SDK...${NC}"
if [ -n "$PICO_SDK_PATH" ] && [ -d "$PICO_SDK_PATH" ]; then
    echo -e "${GREEN}✓ PICO_SDK_PATH found: $PICO_SDK_PATH${NC}"
    PICO_AVAILABLE=true
else
    echo -e "${YELLOW}⚠ PICO_SDK_PATH not set or invalid${NC}"
    echo "Pico W builds will not be available"
    PICO_AVAILABLE=false
fi

# Check ARM toolchain
echo ""
echo -e "${YELLOW}Checking ARM toolchain...${NC}"
if command_exists arm-none-eabi-gcc; then
    GCC_VERSION=$(arm-none-eabi-gcc --version | head -n1)
    echo -e "${GREEN}✓ ARM GCC found: $GCC_VERSION${NC}"
    ARM_AVAILABLE=true
else
    echo -e "${YELLOW}⚠ ARM toolchain not found${NC}"
    echo "STM32 builds will not be available"
    ARM_AVAILABLE=false
fi

# Create corrected CMakeLists.txt
echo ""
echo -e "${YELLOW}Creating corrected CMakeLists.txt...${NC}"

cat > "$PROJECT_ROOT/CMakeLists.txt" << 'EOF'
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
EOF

# Create improved build script
echo -e "${YELLOW}Creating improved build script...${NC}"

cat > "$PROJECT_ROOT/build_improved.sh" << 'EOF'
#!/bin/bash

# Improved build script with better error handling and platform detection

set -e

PLATFORM=${1:-auto}
BUILD_TYPE=${2:-Release}
CLEAN=${3:-false}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Multi-Channel Diagnostic Test Rig Build ===${NC}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Auto-detect platform if not specified
if [ "$PLATFORM" = "auto" ]; then
    if [ -n "$PICO_SDK_PATH" ] && [ -d "$PICO_SDK_PATH" ]; then
        PLATFORM="pico"
        echo -e "${GREEN}Auto-detected Pico W platform${NC}"
    else
        PLATFORM="host"
        echo -e "${YELLOW}Defaulting to host platform${NC}"
    fi
fi

echo "Platform: $PLATFORM"
echo "Build Type: $BUILD_TYPE"

# Clean build directory if requested
if [ "$CLEAN" = "true" ] || [ "$CLEAN" = "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure based on platform
case $PLATFORM in
    "pico"|"pico_w")
        if [ -z "$PICO_SDK_PATH" ]; then
            echo -e "${RED}ERROR: PICO_SDK_PATH not set${NC}"
            echo "Please set PICO_SDK_PATH environment variable"
            exit 1
        fi
        
        echo -e "${BLUE}Building for Raspberry Pi Pico W...${NC}"
        cmake -DTARGET_PLATFORM=pico_w \
              -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
              -DPICO_BOARD=pico_w \
              ..
        ;;
        
    "nucleo"|"stm32")
        echo -e "${BLUE}Building for STM32 Nucleo-F446RE...${NC}"
        if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
            echo -e "${RED}ERROR: ARM toolchain not found${NC}"
            echo "Please install arm-none-eabi-gcc toolchain"
            exit 1
        fi
        
        cmake -DTARGET_PLATFORM=nucleo_f446re \
              -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
              -DCMAKE_TOOLCHAIN_FILE=../platforms/nucleo_f446re/toolchain.cmake \
              ..
        ;;
        
    "host")
        echo -e "${BLUE}Building for host platform...${NC}"
        cmake -DTARGET_PLATFORM=host \
              -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
              ..
        ;;
        
    *)
        echo -e "${RED}ERROR: Unknown platform: $PLATFORM${NC}"
        echo "Usage: $0 [pico|nucleo|host] [Debug|Release] [clean]"
        exit 1
        ;;
esac

# Build
echo -e "${BLUE}Compiling...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "${GREEN}Build completed successfully!${NC}"

# Show output files
echo ""
echo -e "${BLUE}Output files:${NC}"
find . -name "*.uf2" -o -name "*.elf" -o -name "*.bin" -o -name "diagnostic_rig_*" | head -10
EOF

chmod +x "$PROJECT_ROOT/build_improved.sh"

# Create missing stub files for demo and utils
echo ""
echo -e "${YELLOW}Creating missing stub files...${NC}"

# Create hal_demo stub if missing
mkdir -p "$PROJECT_ROOT/src/demo"
if [ ! -f "$PROJECT_ROOT/src/demo/hal_demo.cpp" ] || [ ! -s "$PROJECT_ROOT/src/demo/hal_demo.cpp" ]; then
    cat > "$PROJECT_ROOT/src/demo/hal_demo.cpp" << 'EOF'
#include "demo/hal_demo.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

void run_hal_demo(void) {
    printf("[DEMO] Running HAL demonstration...\n");
    demo_gpio_operations();
    demo_uart_operations(); 
    demo_adc_operations();
    demo_display_operations();
    printf("[DEMO] HAL demonstration complete\n");
}

void demo_gpio_operations(void) {
    printf("[DEMO] GPIO: Blinking LED...\n");
    for (int i = 0; i < 3; i++) {
        hal_gpio_write(25, GPIO_HIGH);
        hal_delay_ms(200);
        hal_gpio_write(25, GPIO_LOW);
        hal_delay_ms(200);
    }
}

void demo_uart_operations(void) {
    printf("[DEMO] UART: Testing communication...\n");
    const char* msg = "HAL Demo UART Test\n";
    hal_uart_transmit(0, (const uint8_t*)msg, strlen(msg), 1000);
}

void demo_adc_operations(void) {
    printf("[DEMO] ADC: Reading channels...\n");
    for (int i = 0; i < 3; i++) {
        uint16_t value;
        if (hal_adc_read(i, &value) == HAL_OK) {
            printf("[DEMO] ADC Ch%d: %u\n", i, value);
        }
    }
}

void demo_display_operations(void) {
    printf("[DEMO] Display: Basic operations...\n");
    hal_display_clear(0x000000);
    hal_display_draw_text(10, 10, "Demo", 0xFFFFFF, 0x000000);
    hal_display_flush();
}
EOF
fi

# Create missing diagnostics engine stub 
mkdir -p "$PROJECT_ROOT/src/monitoring"
if [ ! -f "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" ] || [ ! -s "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" ]; then
    cat > "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" << 'EOF'
#include "monitoring/diagnostics_engine.h"
#include <stdio.h>

void test_diagnostic_channels(void) {
    printf("[DIAG] Testing diagnostic channels...\n");
    // Placeholder for channel testing
}

void toggle_all_channels(void) {
    printf("[DIAG] Toggling all channels\n");
    // Placeholder for channel toggle
}

void get_channel_states(bool* states) {
    // Placeholder - set all channels to enabled for demo
    for (int i = 0; i < 4; i++) {
        states[i] = true;
    }
}

void set_channel_enable(int channel, bool enable) {
    printf("[DIAG] Channel %d %s\n", channel, enable ? "enabled" : "disabled");
}

void print_system_status(void) {
    printf("[DIAG] System Status: Running\n");
}
EOF
fi

# Create missing header stub
if [ ! -f "$PROJECT_ROOT/include/monitoring/diagnostics_engine.h" ] || [ ! -s "$PROJECT_ROOT/include/monitoring/diagnostics_engine.h" ]; then
    mkdir -p "$PROJECT_ROOT/include/monitoring"
    cat > "$PROJECT_ROOT/include/monitoring/diagnostics_engine.h" << 'EOF'
#ifndef DIAGNOSTICS_ENGINE_H
#define DIAGNOSTICS_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_DIAGNOSTIC_CHANNELS 4

void test_diagnostic_channels(void);
void toggle_all_channels(void);
void get_channel_states(bool* states);
void set_channel_enable(int channel, bool enable);
void print_system_status(void);

#ifdef __cplusplus
}
#endif

#endif // DIAGNOSTICS_ENGINE_H
EOF
fi

# Summary
echo ""
echo -e "${GREEN}=== Setup Complete ===${NC}"
echo ""
echo -e "${YELLOW}Available build commands:${NC}"
echo "1. Host build (testing):     ./build_improved.sh host"
echo "2. Pico W build:            ./build_improved.sh pico"
echo "3. STM32 build:             ./build_improved.sh stm32"
echo "4. Clean build:             ./build_improved.sh pico clean"
echo ""

if [ "$PICO_AVAILABLE" = true ]; then
    echo -e "${GREEN}✓ Pico W builds available${NC}"
else
    echo -e "${YELLOW}⚠ To enable Pico W builds:${NC}"
    echo "  export PICO_SDK_PATH=/path/to/pico-sdk"
fi

if [ "$ARM_AVAILABLE" = true ]; then
    echo -e "${GREEN}✓ STM32 builds available${NC}"
else
    echo -e "${YELLOW}⚠ To enable STM32 builds:${NC}"
    echo "  Install ARM toolchain: arm-none-eabi-gcc"
fi

echo ""
echo -e "${BLUE}Next steps:${NC}"
echo "1. Try a host build first: ./build_improved.sh host"
echo "2. If successful, try your target platform"
echo "3. Use the existing flash.sh and monitor.sh scripts"

echo ""
echo -e "${GREEN}Build system setup completed successfully!${NC}"