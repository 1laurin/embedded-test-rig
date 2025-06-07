#!/bin/bash

# Diagnose CMake Include Path Issues
set -e

echo "🔍 Diagnosing CMake Include Path Issues..."
echo ""

PROJECT_ROOT="$(pwd)"
BUILD_DIR="$PROJECT_ROOT/build/pico_w"

echo "📁 Current directory: $PROJECT_ROOT"
echo "📁 Build directory: $BUILD_DIR"
echo ""

echo "🔍 Checking for key files..."

# Check if hal_interface.h exists where it should
echo "Looking for hal_interface.h:"
for location in \
    "include/hal_interface.h" \
    "platforms/common/hal_interface.h" \
    "include/platforms/common/hal_interface.h"
do
    if [ -f "$location" ]; then
        echo "  ✅ Found: $location"
    else
        echo "  ❌ Missing: $location"
    fi
done

echo ""
echo "🔍 Checking include structure:"
if [ -d "include" ]; then
    echo "📁 include/ directory contents:"
    find include -name "*.h" | head -10
else
    echo "❌ include/ directory doesn't exist!"
fi

echo ""
echo "🔍 Current CMakeLists.txt include_directories:"
if [ -f "platforms/pico_w/CMakeLists.txt" ]; then
    grep -A 10 -B 2 "include_directories\|target_include_directories" platforms/pico_w/CMakeLists.txt || echo "No include_directories found"
else
    echo "❌ platforms/pico_w/CMakeLists.txt doesn't exist!"
fi

echo ""
echo "🔧 RECOMMENDED FIX:"
echo "1. Run the fix_cmake_includes.sh script I created"
echo "2. This will create a new CMakeLists.txt with proper include paths"
echo "3. Rebuild: cd build/pico_w && rm -rf * && cmake ../../platforms/pico_w && make"

echo ""
echo "🎯 The core issue is that CMake needs to know where to find header files"
echo "   relative to the build directory, not the source directory."