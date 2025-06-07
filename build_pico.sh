#!/bin/bash

# Pico W Build Helper Script
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/pico_w"

echo "🏗️  Building Pico W Diagnostic Rig..."
echo "Project Root: $PROJECT_ROOT"
echo "Build Dir: $BUILD_DIR"

# Check if PICO_SDK_PATH is set
if [ -z "$PICO_SDK_PATH" ]; then
    echo "❌ ERROR: PICO_SDK_PATH environment variable is not set"
    echo "Please set it to your Pico SDK installation directory"
    echo "Example: export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

echo "✅ PICO_SDK_PATH: $PICO_SDK_PATH"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "🔧 Configuring build..."
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DPICO_BOARD=pico_w \
    -DPICO_PLATFORM=rp2040 \
    "$PROJECT_ROOT/platforms/pico_w"

# Build
echo "🔨 Building..."
make -j$(nproc 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📦 UF2 file: $BUILD_DIR/diagnostic_rig_pico.uf2"
    echo ""
    echo "To flash:"
    echo "1. Hold BOOTSEL button while connecting Pico W to USB"
    echo "2. Copy the UF2 file to the RPI-RP2 drive"
    echo "3. Or run: ./flash.sh pico"
else
    echo "❌ Build failed!"
    exit 1
fi
