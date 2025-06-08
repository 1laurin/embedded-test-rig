#!/bin/bash
# DUPLICATE FILE CONSOLIDATION SCRIPT
# Identifies and helps consolidate duplicate files

set -e

echo "🔄 DUPLICATE FILE CONSOLIDATION"
echo "================================================"

CONSOLIDATION_LOG="consolidation_$(date +%Y%m%d_%H%M%S).log"

log() {
    echo "$1" | tee -a "$CONSOLIDATION_LOG"
}

# =============================================================================
# ANALYZE DUPLICATE FILENAMES
# =============================================================================
echo "🔍 Analyzing duplicate filenames..."

# Find files with same names
duplicate_names=($(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" \) | grep -v build | xargs basename | sort | uniq -d))

if [ ${#duplicate_names[@]} -gt 0 ]; then
    log "Found duplicate filenames:"
    for dup_name in "${duplicate_names[@]}"; do
        log ""
        log "📄 Duplicate filename: $dup_name"
        
        # Find all instances
        instances=($(find . -name "$dup_name" -type f | grep -v build))
        
        for instance in "${instances[@]}"; do
            log "  📁 $instance"
            
            # Get file info
            size=$(wc -c < "$instance" 2>/dev/null || echo "0")
            lines=$(wc -l < "$instance" 2>/dev/null || echo "0")
            log "    Size: $size bytes, Lines: $lines"
            
            # Check for key indicators
            if [[ "$instance" == *main.cpp ]]; then
                if grep -q "cyw43_arch" "$instance"; then
                    log "    ✅ Has WiFi support"
                else
                    log "    ❌ No WiFi support"
                fi
                
                if grep -q "websocket" "$instance"; then
                    log "    ✅ Has WebSocket support"
                else
                    log "    ❌ No WebSocket support"
                fi
            fi
            
            if [[ "$instance" == *CMakeLists.txt ]]; then
                if grep -q "pico_cyw43_arch" "$instance"; then
                    log "    ✅ Has WiFi libraries"
                else
                    log "    ❌ Missing WiFi libraries"
                fi
                
                if grep -q "MEM_LIBC_MALLOC.*0" "$instance"; then
                    log "    ✅ Has memory fix"
                else
                    log "    ❌ Missing memory fix"
                fi
            fi
        done
        
        # Recommendation
        if [[ "$dup_name" == "main.cpp" ]]; then
            log "  💡 RECOMMENDATION: Keep platforms/pico_w/src/main.cpp (with WiFi+WebSocket)"
        elif [[ "$dup_name" == "CMakeLists.txt" ]]; then
            log "  💡 RECOMMENDATION: Keep platforms/pico_w/CMakeLists.txt (with all fixes)"
        elif [[ "$dup_name" == *config* ]]; then
            log "  💡 RECOMMENDATION: Consolidate into single config file with all constants"
        else
            log "  💡 RECOMMENDATION: Review files and keep the most complete version"
        fi
    done
else
    log "✅ No duplicate filenames found"
fi

# =============================================================================
# ANALYZE DUPLICATE CONTENT
# =============================================================================
echo ""
echo "🔍 Analyzing files with identical content..."

# Create temporary file for MD5 analysis
temp_md5_file=$(mktemp)
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" \) | grep -v build | xargs md5sum 2>/dev/null | sort > "$temp_md5_file"

# Find duplicate MD5 hashes
duplicate_hashes=($(awk '{print $1}' "$temp_md5_file" | sort | uniq -d))

if [ ${#duplicate_hashes[@]} -gt 0 ]; then
    log ""
    log "Found files with identical content:"
    
    for hash in "${duplicate_hashes[@]}"; do
        log ""
        log "🔗 Files with identical content (MD5: $hash):"
        
        # Get all files with this hash
        identical_files=($(grep "^$hash" "$temp_md5_file" | cut -d' ' -f2-))
        
        for file in "${identical_files[@]}"; do
            log "  📄 $file"
        done
        
        # If multiple identical files, recommend action
        if [ ${#identical_files[@]} -gt 1 ]; then
            log "  💡 RECOMMENDATION: These files are identical - keep one and remove others"
            
            # Suggest which to keep based on location
            best_file=""
            for file in "${identical_files[@]}"; do
                if [[ "$file" == *platforms/pico_w/* ]]; then
                    best_file="$file"
                    break
                fi
            done
            
            if [ -n "$best_file" ]; then
                log "  🎯 SUGGESTED TO KEEP: $best_file"
            else
                log "  🎯 SUGGESTED TO KEEP: ${identical_files[0]} (first one found)"
            fi
        fi
    done
else
    log "✅ No files with identical content found"
fi

rm -f "$temp_md5_file"

# =============================================================================
# SPECIFIC DUPLICATE SCENARIOS
# =============================================================================
echo ""
echo "🎯 Checking specific duplicate scenarios..."

# Multiple main.cpp files
main_files=($(find . -name "main.cpp" | grep -v build))
if [ ${#main_files[@]} -gt 1 ]; then
    log ""
    log "🔍 Multiple main.cpp files found:"
    
    best_main=""
    for main_file in "${main_files[@]}"; do
        log "  📄 $main_file"
        
        lines=$(wc -l < "$main_file")
        has_wifi=$(grep -l "cyw43_arch" "$main_file" 2>/dev/null || echo "")
        has_websocket=$(grep -l "websocket" "$main_file" 2>/dev/null || echo "")
        
        log "    Lines: $lines"
        
        if [ -n "$has_wifi" ] && [ -n "$has_websocket" ]; then
            log "    ✅ Complete (WiFi + WebSocket)"
            best_main="$main_file"
        elif [ -n "$has_wifi" ]; then
            log "    ⚠️  Has WiFi only"
        elif [ -n "$has_websocket" ]; then
            log "    ⚠️  Has WebSocket only"
        else
            log "    ❌ Basic version"
        fi
    done
    
    if [ -n "$best_main" ]; then
        log "  🎯 KEEP: $best_main"
        log "  🗑️  REMOVE: All others"
    fi
fi

# Multiple CMakeLists.txt files
cmake_files=($(find . -name "CMakeLists.txt" | grep -v build))
if [ ${#cmake_files[@]} -gt 1 ]; then
    log ""
    log "🔍 Multiple CMakeLists.txt files found:"
    
    best_cmake=""
    for cmake_file in "${cmake_files[@]}"; do
        log "  📄 $cmake_file"
        
        has_wifi=$(grep -l "pico_cyw43_arch" "$cmake_file" 2>/dev/null || echo "")
        has_mem_fix=$(grep -l "MEM_LIBC_MALLOC.*0" "$cmake_file" 2>/dev/null || echo "")
        has_websocket=$(grep -l "websocket" "$cmake_file" 2>/dev/null || echo "")
        
        score=0
        if [ -n "$has_wifi" ]; then
            log "    ✅ Has WiFi libraries"
            ((score++))
        fi
        if [ -n "$has_mem_fix" ]; then
            log "    ✅ Has memory fix"
            ((score++))
        fi
        if [ -n "$has_websocket" ]; then
            log "    ✅ Has WebSocket support"
            ((score++))
        fi
        
        log "    Score: $score/3"
        
        if [ $score -eq 3 ]; then
            best_cmake="$cmake_file"
        fi
    done
    
    if [ -n "$best_cmake" ]; then
        log "  🎯 KEEP: $best_cmake"
        log "  🗑️  REMOVE: All others"
    fi
fi

# =============================================================================
# GENERATE CONSOLIDATION SCRIPT
# =============================================================================
echo ""
echo "📝 Generating consolidation script..."

consolidation_script="consolidate_duplicates_$(date +%Y%m%d_%H%M%S).sh"

cat > "$consolidation_script" << 'EOF'
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
EOF

chmod +x "$consolidation_script"

log ""
log "✅ DUPLICATE ANALYSIS COMPLETE!"
log "================================================"
log "📄 Analysis saved to: $CONSOLIDATION_LOG"
log "📝 Consolidation script: $consolidation_script"
log ""
log "🎯 SUMMARY RECOMMENDATIONS:"
log "1. Keep platforms/pico_w/src/main.cpp (most complete)"
log "2. Keep platforms/pico_w/CMakeLists.txt (has all fixes)"
log "3. Consolidate board_config.h files"
log "4. Remove any identical duplicate files"
log "5. Review the generated consolidation script before running"

echo ""
echo "✅ DUPLICATE FILE ANALYSIS COMPLETE!"
echo "Check the log file: $CONSOLIDATION_LOG"