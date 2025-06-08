#!/bin/bash
# DUPLICATE FILE CONSOLIDATION SCRIPT
# Generated automatically - REVIEW BEFORE RUNNING

set -e

echo "🔄 CONSOLIDATING DUPLICATE FILES"
echo "❗ REVIEW EACH SECTION BEFORE UNCOMMENTING"

# =============================================================================
# REMOVE REDUNDANT MAIN.CPP FILES
# =============================================================================
echo "🗑️ Removing redundant main.cpp files..."
# Keep only platforms/pico_w/src/main.cpp
# find . -name "main.cpp" -not -path "./platforms/pico_w/src/*" -delete

# =============================================================================
# REMOVE REDUNDANT CMAKE FILES
# =============================================================================
echo "🗑️ Removing redundant CMakeLists.txt files..."
# Keep only platforms/pico_w/CMakeLists.txt
# find . -name "CMakeLists.txt" -not -path "./platforms/pico_w/*" -delete

# =============================================================================
# CONSOLIDATE CONFIG FILES
# =============================================================================
echo "🔗 Consolidating configuration files..."
# Merge all board_config.h files into platforms/pico_w/include/board_config.h
# Review and merge manually, then remove duplicates
# find . -name "*config*.h" -not -path "./platforms/pico_w/include/*" -delete

# =============================================================================
# REMOVE IDENTICAL FILES
# =============================================================================
echo "🗑️ Removing files with identical content..."
# Based on MD5 analysis above, remove duplicate files
# Example:
# rm ./old_location/duplicate_file.cpp

echo "✅ Consolidation complete!"
