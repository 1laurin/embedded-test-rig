#!/bin/bash

# Build script for multi-channel diagnostic test rig
# Usage: ./build.sh [platform] [options]

set -e

PLATFORM=${1:-pico}
BUILD_TYPE=${2:-Release}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

case $PLATFORM in
    "pico"|"pico_w")
        echo "Building for Raspberry Pi Pico W..."
        "$SCRIPT_DIR/scripts/build/build_pico.sh" "$BUILD_TYPE"
        ;;
    "nucleo"|"stm32")
        echo "Building for STM32 Nucleo-F446RE..."
        "$SCRIPT_DIR/scripts/build/build_nucleo.sh" "$BUILD_TYPE"
        ;;
    *)
        echo "Usage: $0 [pico|nucleo] [Debug|Release]"
        echo "Supported platforms: pico, nucleo"
        exit 1
        ;;
esac

echo "Build completed successfully!"
