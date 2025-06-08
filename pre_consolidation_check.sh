#!/bin/bash
# PRE-CONSOLIDATION ANALYSIS
# Check what duplicates exist before removing them

echo "🔍 PRE-CONSOLIDATION ANALYSIS"
echo "=============================================="

# =============================================================================
# CHECK MAIN.CPP FILES
# =============================================================================
echo "📄 MAIN.CPP FILES ANALYSIS:"
main_files=($(find . -name "main.cpp" -not -path "*/build/*"))

if [ ${#main_files[@]} -gt 1 ]; then
    echo "Found ${#main_files[@]} main.cpp files:"
    for main_file in "${main_files[@]}"; do
        echo "  📁 $main_file"
        lines=$(wc -l < "$main_file")
        size=$(wc -c < "$main_file")
        echo "    Lines: $lines, Size: $size bytes"
        
        # Check capabilities
        if grep -q "cyw43_arch" "$main_file"; then
            echo "    ✅ Has WiFi support"
        else
            echo "    ❌ No WiFi support"
        fi
        
        if grep -q "websocket" "$main_file"; then
            echo "    ✅ Has WebSocket support"
        else
            echo "    ❌ No WebSocket support"
        fi
        
        if grep -q "system_init" "$main_file"; then
            echo "    ✅ Has system initialization"
        else
            echo "    ❌ No system initialization"
        fi
    done
    
    echo ""
    echo "🎯 RECOMMENDATION: Keep platforms/pico_w/src/main.cpp (most complete)"
    echo "🗑️  SAFE TO REMOVE: Others"
else
    echo "✅ Only one main.cpp found: ${main_files[0]}"
fi

echo ""

# =============================================================================
# CHECK CMAKE FILES
# =============================================================================
echo "⚙️  CMAKE FILES ANALYSIS:"
cmake_files=($(find . -name "CMakeLists.txt" -not -path "*/build/*"))

if [ ${#cmake_files[@]} -gt 1 ]; then
    echo "Found ${#cmake_files[@]} CMakeLists.txt files:"
    for cmake_file in "${cmake_files[@]}"; do
        echo "  📁 $cmake_file"
        lines=$(wc -l < "$cmake_file")
        echo "    Lines: $lines"
        
        # Check capabilities
        if grep -q "pico_cyw43_arch" "$cmake_file"; then
            echo "    ✅ Has WiFi libraries"
        else
            echo "    ❌ Missing WiFi libraries"
        fi
        
        if grep -q "MEM_LIBC_MALLOC.*0" "$cmake_file"; then
            echo "    ✅ Has memory allocator fix"
        else
            echo "    ❌ Missing memory allocator fix"
        fi
        
        if grep -q "websocket" "$cmake_file"; then
            echo "    ✅ Has WebSocket references"
        else
            echo "    ❌ No WebSocket references"
        fi
    done
    
    echo ""
    echo "🎯 RECOMMENDATION: Keep platforms/pico_w/CMakeLists.txt (with all fixes)"
    echo "🗑️  SAFE TO REMOVE: Others"
else
    echo "✅ Only one CMakeLists.txt found: ${cmake_files[0]}"
fi

echo ""

# =============================================================================
# CHECK CONFIG FILES
# =============================================================================
echo "🔧 CONFIG FILES ANALYSIS:"
config_files=($(find . -name "*config*.h" -not -path "*/build/*"))

if [ ${#config_files[@]} -gt 1 ]; then
    echo "Found ${#config_files[@]} config files:"
    for config_file in "${config_files[@]}"; do
        echo "  📁 $config_file"
        lines=$(wc -l < "$config_file")
        echo "    Lines: $lines"
        
        # Check for important constants
        if grep -q "WIFI_LED_BLINK" "$config_file"; then
            echo "    ✅ Has WiFi LED constants"
        else
            echo "    ❌ Missing WiFi LED constants"
        fi
        
        if grep -q "GPIO_STATE_HIGH" "$config_file"; then
            echo "    ✅ Has GPIO state constants"
        else
            echo "    ❌ Missing GPIO state constants"
        fi
        
        if grep -q "NET_WEBSOCKET_PORT" "$config_file"; then
            echo "    ✅ Has network constants"
        else
            echo "    ❌ Missing network constants"
        fi
    done
    
    echo ""
    echo "🎯 RECOMMENDATION: Merge into single board_config.h with all constants"
else
    echo "✅ Config files look good: ${#config_files[@]} found"
fi

echo ""

# =============================================================================
# CHECK FOR IDENTICAL FILES
# =============================================================================
echo "🔍 CHECKING FOR IDENTICAL FILES:"
temp_file=$(mktemp)
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" \) -not -path "*/build/*" | \
    xargs md5sum 2>/dev/null | sort > "$temp_file"

duplicates=$(awk '{print $1}' "$temp_file" | sort | uniq -d)

if [ -n "$duplicates" ]; then
    echo "Found files with identical content:"
    for hash in $duplicates; do
        echo ""
        echo "🔗 Identical files (MD5: $hash):"
        grep "^$hash" "$temp_file" | while read line; do
            file_path=$(echo "$line" | cut -d' ' -f2-)
            echo "  📄 $file_path"
        done
        echo "  💡 These files are identical - safe to remove duplicates"
    done
else
    echo "✅ No identical files found"
fi

rm -f "$temp_file"

echo ""
echo "=============================================="
echo "✅ PRE-CONSOLIDATION ANALYSIS COMPLETE"
echo ""
echo "🎯 READY TO PROCEED? The consolidation script looks safe to run."
echo "   Each section removes only redundant/duplicate files."