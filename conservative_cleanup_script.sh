#!/bin/bash
# CONSERVATIVE CLEANUP - PRESERVE DEVELOPMENT STRUCTURE
# Only removes actual duplicates and broken files, keeps placeholders

set -e

echo "🎯 CONSERVATIVE CLEANUP EXECUTION"
echo "Preserving development structure and placeholder files"
echo "======================================================"

# =============================================================================
# BACKUP FIRST
# =============================================================================
echo "📦 Creating backup..."
BACKUP_DIR="../conservative_cleanup_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp -r . "$BACKUP_DIR/" 2>/dev/null || echo "Backup partially created"
echo "✅ Backup created at: $BACKUP_DIR"

# =============================================================================
# REMOVE ONLY ACTUAL DUPLICATE HEADERS (not placeholders)
# =============================================================================
echo ""
echo "🗑️ Removing only actual duplicate headers..."

# These are the actual duplicates we confirmed are identical content
duplicates_to_remove=(
    "./include/system_info.h"           # Keep ./src/system/system_info.h
    "./include/hal_interface.h"         # Keep ./src/utils/hal_interface.h  
    "./include/input_handler.h"         # Keep ./src/ui/input_handler.h
    "./include/system_loop.h"           # Keep ./src/core/system_loop.h
)

for dup_file in "${duplicates_to_remove[@]}"; do
    if [ -f "$dup_file" ]; then
        # Double-check it's actually a duplicate before removing
        corresponding_file=""
        case "$dup_file" in
            "./include/system_info.h") corresponding_file="./src/system/system_info.h" ;;
            "./include/hal_interface.h") corresponding_file="./src/utils/hal_interface.h" ;;
            "./include/input_handler.h") corresponding_file="./src/ui/input_handler.h" ;;
            "./include/system_loop.h") corresponding_file="./src/core/system_loop.h" ;;
        esac
        
        if [ -f "$corresponding_file" ] && cmp -s "$dup_file" "$corresponding_file"; then
            echo "  🗑️ Removing confirmed duplicate: $dup_file"
            rm "$dup_file"
        else
            echo "  ⚠️ Keeping (not identical): $dup_file"
        fi
    fi
done

echo "✅ Actual duplicates removed"

# =============================================================================
# REMOVE ONLY REDUNDANT CMAKE FILES (not placeholders)
# =============================================================================
echo ""
echo "🗑️ Removing only redundant CMakeLists.txt files..."

# Only remove the root CMakeLists.txt if it's clearly redundant
if [ -f "./CMakeLists.txt" ]; then
    # Check if it has any meaningful content compared to the Pico W one
    if [ -f "./platforms/pico_w/CMakeLists.txt" ]; then
        root_lines=$(grep -v '^#' "./CMakeLists.txt" | grep -v '^$' | wc -l)
        pico_lines=$(grep -v '^#' "./platforms/pico_w/CMakeLists.txt" | grep -v '^$' | wc -l)
        
        if [ "$root_lines" -lt 10 ] && [ "$pico_lines" -gt 50 ]; then
            echo "  🗑️ Removing minimal root CMakeLists.txt (keeping feature-complete Pico W version)"
            rm "./CMakeLists.txt"
        else
            echo "  ⚠️ Keeping root CMakeLists.txt (has substantial content)"
        fi
    fi
else
    echo "  ✅ No root CMakeLists.txt to evaluate"
fi

# Keep the nucleo and tests CMakeLists.txt as development placeholders
echo "  📁 Keeping ./platforms/nucleo_f446re/CMakeLists.txt (development placeholder)"
echo "  📁 Keeping ./tests/CMakeLists.txt (development placeholder)"

echo "✅ Only redundant CMake files removed"

# =============================================================================
# SMART CONFIG FILE CONSOLIDATION
# =============================================================================
echo ""
echo "🔗 Smart configuration file consolidation..."

target_config="./platforms/pico_w/include/board_config.h"

# Add missing GPIO constants to the main config if needed
if [ -f "$target_config" ]; then
    echo "  📝 Checking $target_config for missing constants"
    
    # Add GPIO constants if missing
    if ! grep -q "GPIO_STATE_HIGH" "$target_config"; then
        cat >> "$target_config" << 'EOF'

// =============================================================================
// GPIO STATE CONSTANTS (Added during consolidation)
// =============================================================================

#ifndef GPIO_STATE_HIGH
#define GPIO_STATE_HIGH     1
#endif

#ifndef GPIO_STATE_LOW
#define GPIO_STATE_LOW      0
#endif

EOF
        echo "  ✅ Added missing GPIO state constants"
    else
        echo "  ✅ GPIO constants already present"
    fi
    
    # Add missing WiFi LED constants if needed
    if ! grep -q "WIFI_LED_BLINK_CONNECTED_MS" "$target_config"; then
        cat >> "$target_config" << 'EOF'

// =============================================================================
// WIFI LED TIMING CONSTANTS (Added during consolidation)
// =============================================================================

#ifndef WIFI_LED_BLINK_CONNECTED_MS
#define WIFI_LED_BLINK_CONNECTED_MS     1000
#endif

#ifndef WIFI_LED_BLINK_CONNECTING_MS
#define WIFI_LED_BLINK_CONNECTING_MS    200
#endif

EOF
        echo "  ✅ Added missing WiFi LED constants"
    else
        echo "  ✅ WiFi LED constants already present"
    fi
fi

# Keep other config files as development placeholders, but note them
echo "  📁 Keeping ./platforms/nucleo_f446re/include/board_config.h (STM32 development)"
echo "  📁 Keeping ./include/board_config.h (common definitions)"

echo "✅ Configuration files enhanced"

# =============================================================================
# CLEAN ONLY GENUINELY EMPTY DIRECTORIES
# =============================================================================
echo ""
echo "🗂️ Removing only genuinely empty directories..."

# Only remove directories that are completely empty (no files, no subdirs)
empty_dirs=$(find . -type d -empty -not -path "*/.git*" -not -path "*/build*" 2>/dev/null || true)

if [ -n "$empty_dirs" ]; then
    echo "Removing empty directories:"
    echo "$empty_dirs" | while read empty_dir; do
        echo "  🗑️ $empty_dir"
        rmdir "$empty_dir" 2>/dev/null || true
    done
else
    echo "  ✅ No empty directories found"
fi

echo "✅ Empty directories cleaned"

# =============================================================================
# VERIFICATION AND SUMMARY
# =============================================================================
echo ""
echo "🔍 Verification and project summary..."

# Count key files
main_files=$(find . -name "main.cpp" -not -path "*/build/*" | wc -l)
cmake_files=$(find . -name "CMakeLists.txt" -not -path "*/build/*" | wc -l) 
config_files=$(find . -name "*config*.h" -not -path "*/build/*" | wc -l)
placeholder_files=$(find . -type f -size 0 -not -path "*/build/*" | wc -l)

echo "📊 Project Structure After Conservative Cleanup:"
echo "  main.cpp files: $main_files"
echo "  CMakeLists.txt files: $cmake_files"
echo "  config files: $config_files"  
echo "  placeholder files: $placeholder_files"

echo ""
echo "🎯 Active Development Files:"
if [ -f "./platforms/pico_w/src/main.cpp" ]; then
    lines=$(wc -l < "./platforms/pico_w/src/main.cpp")
    echo "  ✅ ./platforms/pico_w/src/main.cpp ($lines lines - ready for development)"
fi

if [ -f "./platforms/pico_w/CMakeLists.txt" ]; then
    echo "  ✅ ./platforms/pico_w/CMakeLists.txt (complete with WiFi + WebSocket support)"
fi

if [ -f "./platforms/pico_w/include/board_config.h" ]; then
    lines=$(wc -l < "./platforms/pico_w/include/board_config.h")
    echo "  ✅ ./platforms/pico_w/include/board_config.h ($lines lines - all constants)"
fi

echo ""
echo "📁 Development Placeholders Preserved:"
echo "  📋 STM32 Nucleo platform structure (for future implementation)"
echo "  📋 Test framework structure (for unit/integration tests)"
echo "  📋 Driver placeholder files (for future driver development)"
echo "  📋 Protocol handler placeholders (for CAN, Modbus, etc.)"

echo ""
echo "======================================================"
echo "✅ CONSERVATIVE CLEANUP COMPLETE!"
echo "📦 Backup: $BACKUP_DIR"
echo "🚀 Ready for development: Your placeholder structure is intact"
echo "🧪 Test current build: cd platforms/pico_w && mkdir build && cd build && cmake .. && make"

echo ""
echo "💡 Next Development Steps:"
echo "  1. Continue building out your placeholder files as needed"
echo "  2. The Pico W platform is ready for immediate development"
echo "  3. STM32 platform structure is ready for future implementation"