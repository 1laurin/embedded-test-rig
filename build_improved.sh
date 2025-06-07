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
