#!/bin/bash

# Quick debug to see exactly what's happening
set -e

echo "🔍 Debugging build issues..."

# Check if our function implementations exist
echo "📝 Checking if functions are implemented..."

echo "=== Checking for request_system_stop ==="
grep -n "request_system_stop" src/core/system_loop.cpp || echo "❌ NOT FOUND in system_loop.cpp"

echo "=== Checking for input_handler_init ==="  
grep -n "input_handler_init" src/ui/input_handler.cpp || echo "❌ NOT FOUND in input_handler.cpp"

echo "=== Checking for run_main_loop ==="
grep -n "run_main_loop" src/core/system_loop.cpp || echo "❌ NOT FOUND in system_loop.cpp"

echo ""
echo "📁 Checking which files CMake is actually compiling..."
cd build/pico_w
make VERBOSE=1 2>&1 | grep "src.*\.cpp" | head -10

echo ""
echo "🔗 Checking what the linker sees..."
make 2>&1 | grep "undefined reference" | head -5

echo ""
echo "🎯 Quick fix: Let's make sure all our source files are in the CMake build..."

# Go back to project root and check our actual source files
cd ../..

echo "Files in src/core/:"
ls -la src/core/*.cpp 2>/dev/null || echo "No .cpp files in src/core/"

echo "Files in src/ui/:"  
ls -la src/ui/*.cpp 2>/dev/null || echo "No .cpp files in src/ui/"

echo "Files in src/system/:"
ls -la src/system/*.cpp 2>/dev/null || echo "No .cpp files in src/system/"

echo ""
echo "💡 RECOMMENDATION:"
echo "The issue might be that CMake isn't finding our source files."
echo "Run: ./fix_main_and_rebuild.sh"
echo "This will clean rebuild everything and should fix the linking."