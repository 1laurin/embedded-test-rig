#!/bin/bash
# HEADER FILE ORGANIZATION SCRIPT
# Moves headers to appropriate include directories

set -e

echo "📂 ORGANIZING HEADER FILES"
echo "================================================"

# =============================================================================
# CREATE PROPER DIRECTORY STRUCTURE
# =============================================================================
echo "📁 Creating proper include directory structure..."

# Create main include directories
mkdir -p include
mkdir -p platforms/pico_w/include

# Create module-specific include directories
mkdir -p src/core
mkdir -p src/ui  
mkdir -p src/system
mkdir -p src/monitoring
mkdir -p src/utils
mkdir -p src/demo

echo "✅ Directory structure created"

# =============================================================================
# ANALYZE CURRENT HEADER LOCATIONS
# =============================================================================
echo ""
echo "🔍 Analyzing current header file locations..."

echo "Current header files found:"
find . -name "*.h" -not -path "*/build/*" -not -path "*/.git/*" | while read header; do
    echo "  📄 $header"
done

# =============================================================================
# ORGANIZE HEADERS BY CATEGORY
# =============================================================================
echo ""
echo "🗂️ Organizing headers by category..."

# Function to move header and update any corresponding source file
move_header() {
    local source_header="$1"
    local dest_dir="$2"
    local header_name=$(basename "$source_header")
    local dest_path="$dest_dir/$header_name"
    
    if [ -f "$source_header" ]; then
        # Check if source and destination are the same
        if [ "$source_header" = "$dest_path" ] || [ "$(realpath "$source_header" 2>/dev/null)" = "$(realpath "$dest_path" 2>/dev/null)" ]; then
            echo "  ✅ Already in correct location: $source_header"
            return 0
        fi
        
        echo "  Moving: $source_header -> $dest_path"
        mkdir -p "$dest_dir"
        
        # If destination already exists and is different, handle it
        if [ -f "$dest_path" ]; then
            echo "    ⚠️  Destination exists, comparing..."
            if cmp -s "$source_header" "$dest_path"; then
                echo "    ✅ Files are identical, removing source"
                rm "$source_header"
            else
                echo "    ❌ Files differ, keeping both (renaming source)"
                local backup_name="${dest_path%.h}_backup_$(date +%s).h"
                mv "$source_header" "$backup_name"
                echo "    📁 Source renamed to: $backup_name"
            fi
        else
            mv "$source_header" "$dest_path"
        fi
        
        # Check if there's a corresponding .cpp file
        local cpp_file="${source_header%.h}.cpp"
        local c_file="${source_header%.h}.c"
        
        if [ -f "$cpp_file" ]; then
            local cpp_dest_dir="${dest_dir/include/src}"
            local cpp_dest_path="$cpp_dest_dir/$(basename "$cpp_file")"
            
            # Check if cpp source and destination are the same
            if [ "$cpp_file" = "$cpp_dest_path" ] || [ "$(realpath "$cpp_file" 2>/dev/null)" = "$(realpath "$cpp_dest_path" 2>/dev/null)" ]; then
                echo "    ✅ CPP already in correct location: $cpp_file"
            else
                mkdir -p "$cpp_dest_dir"
                if [ -f "$cpp_dest_path" ]; then
                    if cmp -s "$cpp_file" "$cpp_dest_path"; then
                        echo "    ✅ CPP files identical, removing source"
                        rm "$cpp_file"
                    else
                        local cpp_backup="${cpp_dest_path%.cpp}_backup_$(date +%s).cpp"
                        mv "$cpp_file" "$cpp_backup"
                        echo "    📁 CPP renamed to: $cpp_backup"
                    fi
                else
                    echo "    Also moving: $cpp_file -> $cpp_dest_path"
                    mv "$cpp_file" "$cpp_dest_path"
                fi
            fi
        fi
        
        if [ -f "$c_file" ]; then
            local c_dest_dir="${dest_dir/include/src}"
            local c_dest_path="$c_dest_dir/$(basename "$c_file")"
            
            # Check if c source and destination are the same
            if [ "$c_file" = "$c_dest_path" ] || [ "$(realpath "$c_file" 2>/dev/null)" = "$(realpath "$c_dest_path" 2>/dev/null)" ]; then
                echo "    ✅ C file already in correct location: $c_file"
            else
                mkdir -p "$c_dest_dir"
                if [ -f "$c_dest_path" ]; then
                    if cmp -s "$c_file" "$c_dest_path"; then
                        echo "    ✅ C files identical, removing source"
                        rm "$c_file"
                    else
                        local c_backup="${c_dest_path%.c}_backup_$(date +%s).c"
                        mv "$c_file" "$c_backup"
                        echo "    📁 C file renamed to: $c_backup"
                    fi
                else
                    echo "    Also moving: $c_file -> $c_dest_path"
                    mv "$c_file" "$c_dest_path"
                fi
            fi
        fi
    fi
}

# Organize by header name patterns
echo "Organizing system headers..."
find . -name "*system*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/system"
done

echo "Organizing core headers..."
find . -name "*init*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/core"
done

find . -name "*loop*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/core"
done

echo "Organizing UI headers..."
find . -name "*input*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/ui"
done

echo "Organizing monitoring headers..."
find . -name "*monitor*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/monitoring"
done

find . -name "*diagnostic*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/monitoring"
done

echo "Organizing safety headers..."
find . -name "*safety*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/system"
done

echo "Organizing HAL headers..."
find . -name "*hal*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/utils"
done

echo "Organizing demo headers..."
find . -name "*demo*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "src/demo"
done

echo "Organizing Pico W specific headers..."
find . -name "*websocket*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "platforms/pico_w/include"
done

find . -name "*wifi*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "platforms/pico_w/include"
done

echo "Organizing board configuration..."
find . -name "*config*" -name "*.h" -not -path "*/include/*" -not -path "*/build/*" | while read header; do
    move_header "$header" "platforms/pico_w/include"
done

# =============================================================================
# HANDLE REMAINING HEADERS
# =============================================================================
echo ""
echo "📋 Handling remaining headers..."

remaining_headers=$(find . -name "*.h" -not -path "*/include/*" -not -path "*/src/*" -not -path "*/platforms/*" -not -path "*/build/*" -not -path "*/.git/*" 2>/dev/null || true)

if [ -n "$remaining_headers" ]; then
    echo "Moving remaining headers to main include directory:"
    echo "$remaining_headers" | while read header; do
        move_header "$header" "include"
    done
else
    echo "No remaining headers to organize"
fi

# =============================================================================
# UPDATE INCLUDE PATHS IN SOURCE FILES
# =============================================================================
echo ""
echo "🔧 Updating include paths in source files..."

# Function to update include paths
update_includes() {
    local file="$1"
    echo "  Updating includes in: $file"
    
    # Update common header includes
    sed -i.bak 's|#include "system_|#include "../system/system_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "input_|#include "../ui/input_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "safety_|#include "../system/safety_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "hal_|#include "../utils/hal_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "diagnostics_|#include "../monitoring/diagnostics_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "board_config.h"|#include "../include/board_config.h"|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "websocket_|#include "../include/websocket_|g' "$file" 2>/dev/null || true
    sed -i.bak 's|#include "wifi_|#include "../include/wifi_|g' "$file" 2>/dev/null || true
    
    # Remove backup files
    rm -f "$file.bak" 2>/dev/null || true
}

# Update includes in source files
find . -name "*.cpp" -o -name "*.c" | grep -v build | while read source_file; do
    update_includes "$source_file"
done

echo "✅ Include paths updated"

# =============================================================================
# VERIFY ORGANIZATION
# =============================================================================
echo ""
echo "🔍 Verifying header organization..."

echo "Headers now organized as:"
echo "📁 Main include/:"
find include -name "*.h" 2>/dev/null | while read h; do echo "  📄 $h"; done

echo "📁 Platform-specific include/:"
find platforms/*/include -name "*.h" 2>/dev/null | while read h; do echo "  📄 $h"; done

echo "📁 Module-specific headers:"
find src/*/include -name "*.h" 2>/dev/null | while read h; do echo "  📄 $h"; done

orphaned_headers=$(find . -name "*.h" -not -path "*/include/*" -not -path "*/src/*" -not -path "*/platforms/*" -not -path "*/build/*" -not -path "*/.git/*" 2>/dev/null || true)
if [ -n "$orphaned_headers" ]; then
    echo "⚠️ Orphaned headers still found:"
    echo "$orphaned_headers"
else
    echo "✅ All headers properly organized"
fi

echo ""
echo "✅ HEADER ORGANIZATION COMPLETE!"
echo "================================================"
echo "Next: Review and test the build system"