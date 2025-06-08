#!/bin/bash

# =============================================================================
# PROJECT STRUCTURE AUDIT SCRIPT
# Multi-Channel Diagnostic Test Rig - Cleanup and Analysis Tool
# =============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
PROJECT_ROOT="$(pwd)"
AUDIT_OUTPUT_DIR="${PROJECT_ROOT}/audit_results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
AUDIT_LOG="${AUDIT_OUTPUT_DIR}/audit_${TIMESTAMP}.log"

# Ensure audit directory exists
mkdir -p "${AUDIT_OUTPUT_DIR}"

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

log() {
    echo -e "$1" | tee -a "${AUDIT_LOG}"
}

section_header() {
    log "\n${CYAN}================================================================================================${NC}"
    log "${CYAN}$1${NC}"
    log "${CYAN}================================================================================================${NC}"
}

subsection_header() {
    log "\n${BLUE}--- $1 ---${NC}"
}

warning() {
    log "${YELLOW}⚠️  WARNING: $1${NC}"
}

error() {
    log "${RED}❌ ERROR: $1${NC}"
}

success() {
    log "${GREEN}✅ $1${NC}"
}

info() {
    log "${PURPLE}ℹ️  $1${NC}"
}

# =============================================================================
# AUDIT FUNCTIONS
# =============================================================================

audit_project_structure() {
    section_header "PROJECT STRUCTURE ANALYSIS"
    
    # Generate tree structure
    subsection_header "Directory Tree"
    if command -v tree >/dev/null 2>&1; then
        tree -I 'build*|.git|node_modules|*.o|*.elf|*.bin|*.uf2' "${PROJECT_ROOT}" | tee -a "${AUDIT_LOG}"
    else
        find "${PROJECT_ROOT}" -type d | grep -v -E '(build|\.git|node_modules)' | sort | tee -a "${AUDIT_LOG}"
    fi
    
    # Count files by type
    subsection_header "File Type Analysis"
    log "File counts by extension:"
    find "${PROJECT_ROOT}" -type f -name "*.*" | grep -v -E '(build/|\.git/|node_modules/)' | \
        sed 's/.*\.//' | sort | uniq -c | sort -nr | head -20 | tee -a "${AUDIT_LOG}"
}

find_duplicate_files() {
    section_header "DUPLICATE FILE DETECTION"
    
    subsection_header "Duplicate Files by Name"
    find "${PROJECT_ROOT}" -type f -name "*.cpp" -o -name "*.h" -o -name "*.c" | \
        grep -v -E '(build/|\.git/)' | \
        xargs -I{} basename {} | sort | uniq -d | while read filename; do
            warning "Duplicate filename: $filename"
            find "${PROJECT_ROOT}" -name "$filename" -type f | grep -v -E '(build/|\.git/)' | \
                while read filepath; do
                    log "  📁 $filepath"
                done
        done
    
    subsection_header "Duplicate Files by Content (MD5)"
    temp_md5_file=$(mktemp)
    find "${PROJECT_ROOT}" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.c" \) | \
        grep -v -E '(build/|\.git/)' | \
        xargs md5sum 2>/dev/null | sort > "$temp_md5_file"
    
    awk '{print $1}' "$temp_md5_file" | sort | uniq -d | while read hash; do
        warning "Files with identical content (MD5: $hash):"
        grep "^$hash" "$temp_md5_file" | while read line; do
            filepath=$(echo "$line" | cut -d' ' -f2-)
            log "  📄 $filepath"
        done
    done
    rm -f "$temp_md5_file"
}

find_redundant_cmakes() {
    section_header "CMAKE FILES ANALYSIS"
    
    subsection_header "All CMakeLists.txt Files"
    find "${PROJECT_ROOT}" -name "CMakeLists.txt" | grep -v build | while read cmake_file; do
        log "📄 $cmake_file"
        
        # Check for common issues
        if grep -q "pico_cyw43_arch_lwip_threadsafe_background" "$cmake_file"; then
            success "  ✅ Has WiFi libraries"
        else
            warning "  ⚠️  Missing WiFi libraries"
        fi
        
        if grep -q "PICO_BOARD.*pico_w" "$cmake_file"; then
            success "  ✅ Sets PICO_BOARD=pico_w"
        else
            warning "  ⚠️  Missing PICO_BOARD=pico_w"
        fi
        
        if grep -q "MEM_LIBC_MALLOC.*0" "$cmake_file"; then
            success "  ✅ Fixes memory allocator"
        else
            warning "  ⚠️  Missing memory allocator fix"
        fi
    done
}

find_redundant_includes() {
    section_header "INCLUDE DIRECTORY ANALYSIS"
    
    subsection_header "All Include Directories"
    find "${PROJECT_ROOT}" -type d -name "include" | grep -v build | while read include_dir; do
        log "📁 $include_dir"
        file_count=$(find "$include_dir" -name "*.h" | wc -l)
        log "  📄 Contains $file_count header files"
        
        if [ $file_count -eq 0 ]; then
            warning "  ⚠️  Empty include directory"
        fi
    done
    
    subsection_header "Header Files Outside Include Directories"
    find "${PROJECT_ROOT}" -name "*.h" | grep -v -E '(build/|include/|\.git/)' | while read header; do
        warning "Header file outside include directory: $header"
    done
}

analyze_build_artifacts() {
    section_header "BUILD ARTIFACTS ANALYSIS"
    
    subsection_header "Build Directories"
    find "${PROJECT_ROOT}" -type d -name "build*" | while read build_dir; do
        size=$(du -sh "$build_dir" 2>/dev/null | cut -f1)
        log "📁 $build_dir (Size: $size)"
        
        # Check age
        if [ -d "$build_dir" ]; then
            age_days=$(find "$build_dir" -maxdepth 1 -type f -mtime +7 | wc -l)
            if [ $age_days -gt 0 ]; then
                warning "  ⚠️  Contains files older than 7 days - consider cleaning"
            fi
        fi
    done
    
    subsection_header "Scattered Build Artifacts"
    find "${PROJECT_ROOT}" -name "*.o" -o -name "*.elf" -o -name "*.bin" -o -name "*.uf2" | \
        grep -v build | while read artifact; do
        warning "Build artifact outside build directory: $artifact"
    done
}

check_redundant_source_files() {
    section_header "SOURCE FILE REDUNDANCY ANALYSIS"
    
    subsection_header "Multiple main.cpp Files"
    find "${PROJECT_ROOT}" -name "main.cpp" | grep -v build | while read main_file; do
        log "📄 $main_file"
        
        # Check if it includes WiFi headers
        if grep -q "pico/cyw43_arch.h" "$main_file"; then
            success "  ✅ Has WiFi includes"
        else
            info "  ℹ️  No WiFi includes (might be for regular Pico)"
        fi
        
        # Count lines
        lines=$(wc -l < "$main_file")
        log "  📏 $lines lines"
    done
    
    subsection_header "HAL Implementation Files"
    find "${PROJECT_ROOT}" -name "*hal*" -type f | grep -v build | sort | while read hal_file; do
        log "📄 $hal_file"
    done
    
    subsection_header "WebSocket Implementation Files"
    find "${PROJECT_ROOT}" -name "*websocket*" -type f | grep -v build | sort | while read ws_file; do
        log "📄 $ws_file"
    done
}

analyze_config_files() {
    section_header "CONFIGURATION FILES ANALYSIS"
    
    subsection_header "Board Configuration Files"
    find "${PROJECT_ROOT}" -name "*config*" -type f | grep -v build | while read config_file; do
        log "📄 $config_file"
        
        if [[ "$config_file" == *board_config* ]]; then
            # Check for common constants
            if grep -q "WIFI_LED_BLINK" "$config_file"; then
                success "  ✅ Has WiFi LED constants"
            else
                warning "  ⚠️  Missing WiFi LED constants"
            fi
            
            if grep -q "GPIO_STATE_HIGH" "$config_file"; then
                success "  ✅ Has GPIO state constants"
            else
                warning "  ⚠️  Missing GPIO state constants"
            fi
        fi
    done
}

generate_cleanup_recommendations() {
    section_header "CLEANUP RECOMMENDATIONS"
    
    local cleanup_script="${AUDIT_OUTPUT_DIR}/cleanup_recommendations_${TIMESTAMP}.sh"
    
    cat > "$cleanup_script" << 'EOF'
#!/bin/bash
# Generated cleanup recommendations
# Review each command before executing!

set -e

echo "🧹 PROJECT CLEANUP SCRIPT"
echo "Review each section before uncommenting and running"

# =============================================================================
# REMOVE OLD BUILD ARTIFACTS
# =============================================================================
echo "Removing old build directories..."
# find . -name "build*" -type d -exec rm -rf {} \; 2>/dev/null || true

# =============================================================================
# REMOVE SCATTERED BUILD ARTIFACTS  
# =============================================================================
echo "Removing scattered build artifacts..."
# find . -name "*.o" -not -path "*/build/*" -delete
# find . -name "*.elf" -not -path "*/build/*" -delete
# find . -name "*.bin" -not -path "*/build/*" -delete

# =============================================================================
# ORGANIZE HEADER FILES
# =============================================================================
echo "TODO: Move header files to appropriate include directories"

# =============================================================================
# CONSOLIDATE DUPLICATE FILES
# =============================================================================
echo "TODO: Review and merge duplicate files identified in audit"

echo "✅ Cleanup complete!"
EOF
    
    chmod +x "$cleanup_script"
    success "Generated cleanup script: $cleanup_script"
    
    subsection_header "Recommended Actions"
    log "1. 🗂️  Consolidate duplicate files found above"
    log "2. 🧹 Clean old build directories ($(find "${PROJECT_ROOT}" -name "build*" -type d | wc -l) found)"
    log "3. 📁 Move scattered header files to include directories"
    log "4. ⚙️  Use the single corrected CMakeLists.txt for Pico W"
    log "5. 🔧 Remove redundant configuration files"
    log "6. 📝 Update documentation to reflect current structure"
}

create_recommended_structure() {
    section_header "RECOMMENDED PROJECT STRUCTURE"
    
    local structure_file="${AUDIT_OUTPUT_DIR}/recommended_structure.txt"
    
    cat > "$structure_file" << 'EOF'
scope_rig/
├── src/
│   ├── core/           # System initialization and main loop
│   ├── ui/             # Input handling
│   ├── system/         # Safety monitoring
│   ├── monitoring/     # Diagnostics engine
│   ├── utils/          # Utilities and HAL testing
│   └── demo/           # Demo code
├── include/            # Common headers
├── platforms/
│   ├── pico_w/
│   │   ├── src/        # Platform-specific sources (main.cpp, websocket, wifi)
│   │   ├── include/    # Platform-specific headers
│   │   ├── hal/        # Hardware abstraction layer
│   │   └── CMakeLists.txt
│   └── nucleo_f446re/  # Future STM32 target
├── web/                # Web interface simulation
├── tests/
│   ├── unit/
│   ├── integration/
│   └── mocks/
├── scripts/            # Build and utility scripts
└── docs/               # Documentation
EOF
    
    success "Recommended structure saved to: $structure_file"
}

# =============================================================================
# MAIN EXECUTION
# =============================================================================

main() {
    log "${GREEN}🔍 PROJECT AUDIT STARTED at $(date)${NC}"
    log "📁 Project root: ${PROJECT_ROOT}"
    log "📄 Audit log: ${AUDIT_LOG}"
    
    audit_project_structure
    find_duplicate_files
    find_redundant_cmakes
    find_redundant_includes
    analyze_build_artifacts
    check_redundant_source_files
    analyze_config_files
    generate_cleanup_recommendations
    create_recommended_structure
    
    section_header "AUDIT SUMMARY"
    log "📊 Audit completed successfully!"
    log "📁 Results saved in: ${AUDIT_OUTPUT_DIR}"
    log "📄 Detailed log: ${AUDIT_LOG}"
    log "🧹 Cleanup script: ${AUDIT_OUTPUT_DIR}/cleanup_recommendations_${TIMESTAMP}.sh"
    log "📋 Recommended structure: ${AUDIT_OUTPUT_DIR}/recommended_structure.txt"
    
    success "Run the cleanup script after reviewing the recommendations!"
}

# Run the audit
main "$@"