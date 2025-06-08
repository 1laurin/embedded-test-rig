#!/bin/bash

# =============================================================================
# REDUNDANCY DETECTION AND CLEANUP SCRIPT
# Specifically targets duplicate processes, configs, and build systems
# =============================================================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

PROJECT_ROOT="$(pwd)"
CLEANUP_LOG="redundancy_cleanup_$(date +%Y%m%d_%H%M%S).log"

log() {
    echo -e "$1" | tee -a "${CLEANUP_LOG}"
}

section() {
    log "\n${CYAN}========================================${NC}"
    log "${CYAN}$1${NC}"
    log "${CYAN}========================================${NC}"
}

# =============================================================================
# DUPLICATE CMAKE DETECTION
# =============================================================================

detect_cmake_redundancy() {
    section "DUPLICATE CMAKE CONFIGURATIONS"
    
    local cmake_files=($(find "${PROJECT_ROOT}" -name "CMakeLists.txt" | grep -v build))
    
    if [ ${#cmake_files[@]} -gt 1 ]; then
        log "${YELLOW}Found ${#cmake_files[@]} CMakeLists.txt files:${NC}"
        for cmake_file in "${cmake_files[@]}"; do
            log "📄 $cmake_file"
            
            # Analyze each CMakeLists.txt
            local has_wifi=$(grep -l "pico_cyw43_arch" "$cmake_file" 2>/dev/null || echo "")
            local has_websocket=$(grep -l "websocket" "$cmake_file" 2>/dev/null || echo "")
            local has_mem_fix=$(grep -l "MEM_LIBC_MALLOC.*0" "$cmake_file" 2>/dev/null || echo "")
            
            if [ -n "$has_wifi" ]; then
                log "  ✅ Has WiFi support"
            else
                log "  ❌ Missing WiFi support"
            fi
            
            if [ -n "$has_websocket" ]; then
                log "  ✅ Has WebSocket support"  
            else
                log "  ❌ Missing WebSocket support"
            fi
            
            if [ -n "$has_mem_fix" ]; then
                log "  ✅ Has memory allocator fix"
            else
                log "  ❌ Missing memory allocator fix"
            fi
        done
        
        # Recommend which to keep
        log "\n${GREEN}RECOMMENDATION:${NC}"
        log "Keep only: platforms/pico_w/CMakeLists.txt (with all fixes)"
        log "Consider removing redundant CMakeLists.txt files in other locations"
    fi
}

# =============================================================================
# DUPLICATE MAIN.CPP DETECTION
# =============================================================================

detect_main_redundancy() {
    section "DUPLICATE MAIN.CPP FILES"
    
    local main_files=($(find "${PROJECT_ROOT}" -name "main.cpp" | grep -v build))
    
    if [ ${#main_files[@]} -gt 1 ]; then
        log "${YELLOW}Found ${#main_files[@]} main.cpp files:${NC}"
        
        for main_file in "${main_files[@]}"; do
            log "📄 $main_file"
            
            local lines=$(wc -l < "$main_file")
            local has_wifi=$(grep -l "cyw43_arch" "$main_file" 2>/dev/null || echo "")
            local has_websocket=$(grep -l "websocket" "$main_file" 2>/dev/null || echo "")
            
            log "  📏 $lines lines"
            
            if [ -n "$has_wifi" ]; then
                log "  ✅ Has WiFi code"
            else
                log "  ❌ No WiFi code"
            fi
            
            if [ -n "$has_websocket" ]; then
                log "  ✅ Has WebSocket code"
            else
                log "  ❌ No WebSocket code"
            fi
        done
        
        log "\n${GREEN}RECOMMENDATION:${NC}"
        log "Keep only: platforms/pico_w/src/main.cpp (with WiFi + WebSocket)"
        log "Archive or remove other main.cpp files"
    fi
}

# =============================================================================
# DUPLICATE BUILD SYSTEMS
# =============================================================================

detect_build_redundancy() {
    section "DUPLICATE BUILD SYSTEMS AND CONFIGURATIONS"
    
    # Find different build system files
    local makefiles=($(find "${PROJECT_ROOT}" -name "Makefile" | grep -v build))
    local build_scripts=($(find "${PROJECT_ROOT}" -name "build.sh" -o -name "*.mk"))
    
    if [ ${#makefiles[@]} -gt 0 ]; then
        log "${YELLOW}Found Makefiles (potentially redundant with CMake):${NC}"
        for makefile in "${makefiles[@]}"; do
            log "📄 $makefile"
        done
        log "${GREEN}RECOMMENDATION: Remove Makefiles if using CMake${NC}"
    fi
    
    if [ ${#build_scripts[@]} -gt 0 ]; then
        log "\n${YELLOW}Found build scripts:${NC}"
        for script in "${build_scripts[@]}"; do
            log "📄 $script"
        done
    fi
}

# =============================================================================
# CONFIGURATION FILE REDUNDANCY
# =============================================================================

detect_config_redundancy() {
    section "CONFIGURATION FILE REDUNDANCY"
    
    # Find all configuration files
    local config_files=($(find "${PROJECT_ROOT}" -name "*config*" -type f | grep -v build))
    local board_configs=($(find "${PROJECT_ROOT}" -name "board_config*" -type f | grep -v build))
    
    if [ ${#board_configs[@]} -gt 1 ]; then
        log "${YELLOW}Found multiple board_config files:${NC}"
        for config in "${board_configs[@]}"; do
            log "📄 $config"
            
            # Check for key constants
            local has_wifi_constants=$(grep -l "WIFI_LED_BLINK" "$config" 2>/dev/null || echo "")
            local has_gpio_constants=$(grep -l "GPIO_STATE_HIGH" "$config" 2>/dev/null || echo "")
            
            if [ -n "$has_wifi_constants" ]; then
                log "  ✅ Has WiFi constants"
            else
                log "  ❌ Missing WiFi constants"
            fi
            
            if [ -n "$has_gpio_constants" ]; then
                log "  ✅ Has GPIO constants"
            else
                log "  ❌ Missing GPIO constants"
            fi
        done
        
        log "\n${GREEN}RECOMMENDATION:${NC}"
        log "Consolidate into single board_config.h with all required constants"
    fi
}

# =============================================================================
# REDUNDANT HEADER FILES
# =============================================================================

detect_header_redundancy() {
    section "REDUNDANT HEADER FILES"
    
    # Find headers with same names in different locations
    local header_basenames=($(find "${PROJECT_ROOT}" -name "*.h" | grep -v build | xargs basename | sort | uniq -d))
    
    if [ ${#header_basenames[@]} -gt 0 ]; then
        log "${YELLOW}Found duplicate header filenames:${NC}"
        for header_name in "${header_basenames[@]}"; do
            log "\n📄 $header_name found in:"
            find "${PROJECT_ROOT}" -name "$header_name" | grep -v build | while read header_path; do
                log "  📁 $header_path"
            done
        done
        
        log "\n${GREEN}RECOMMENDATION:${NC}"
        log "Consolidate duplicate headers or ensure they serve different purposes"
    fi
}

# =============================================================================
# REDUNDANT PROCESSES (WEBSOCKET/WIFI IMPLEMENTATIONS)
# =============================================================================

detect_process_redundancy() {
    section "REDUNDANT IMPLEMENTATION FILES"
    
    # WebSocket implementations
    local websocket_files=($(find "${PROJECT_ROOT}" -name "*websocket*" | grep -v build))
    if [ ${#websocket_files[@]} -gt 0 ]; then
        log "${YELLOW}WebSocket implementation files:${NC}"
        for ws_file in "${websocket_files[@]}"; do
            log "📄 $ws_file"
        done
        
        if [ ${#websocket_files[@]} -gt 2 ]; then
            log "${GREEN}RECOMMENDATION: Should have only websocket_server.h and websocket_server.cpp${NC}"
        fi
    fi
    
    # WiFi implementations  
    local wifi_files=($(find "${PROJECT_ROOT}" -name "*wifi*" | grep -v build))
    if [ ${#wifi_files[@]} -gt 0 ]; then
        log "\n${YELLOW}WiFi implementation files:${NC}"
        for wifi_file in "${wifi_files[@]}"; do
            log "📄 $wifi_file"
        done
        
        if [ ${#wifi_files[@]} -gt 2 ]; then
            log "${GREEN}RECOMMENDATION: Should have only wifi_manager.h and wifi_manager.cpp${NC}"
        fi
    fi
}

# =============================================================================
# GENERATE CLEANUP SCRIPT
# =============================================================================

generate_cleanup_script() {
    section "GENERATING CLEANUP SCRIPT"
    
    local cleanup_script="auto_cleanup_$(date +%Y%m%d_%H%M%S).sh"
    
    cat > "$cleanup_script" << 'EOF'
#!/bin/bash
# AUTO-GENERATED CLEANUP SCRIPT
# REVIEW BEFORE RUNNING!

set -e

echo "🧹 AUTOMATED CLEANUP SCRIPT"
echo "❗ REVIEW EACH SECTION BEFORE UNCOMMENTING"

# =============================================================================
# REMOVE REDUNDANT CMAKE FILES (keep only platforms/pico_w/CMakeLists.txt)
# =============================================================================
echo "🗂️  Removing redundant CMakeLists.txt files..."
# find . -name "CMakeLists.txt" -not -path "./platforms/pico_w/*" -delete

# =============================================================================
# REMOVE REDUNDANT MAIN FILES (keep only platforms/pico_w/src/main.cpp)
# =============================================================================
echo "🗂️  Removing redundant main.cpp files..."
# find . -name "main.cpp" -not -path "./platforms/pico_w/src/*" -delete

# =============================================================================
# CLEAN BUILD ARTIFACTS
# =============================================================================
echo "🧹 Cleaning build artifacts..."
# find . -name "build*" -type d -exec rm -rf {} \; 2>/dev/null || true
# find . -name "*.o" -delete 2>/dev/null || true
# find . -name "*.elf" -delete 2>/dev/null || true
# find . -name "*.bin" -delete 2>/dev/null || true

# =============================================================================
# ORGANIZE STRUCTURE
# =============================================================================
echo "📁 Creating recommended directory structure..."
# mkdir -p platforms/pico_w/{src,include,hal}
# mkdir -p {src/{core,ui,system,monitoring,utils,demo},include,tests/{unit,integration,mocks}}

echo "✅ Cleanup complete! Review the changes and rebuild."
EOF
    
    chmod +x "$cleanup_script"
    log "${GREEN}Generated cleanup script: $cleanup_script${NC}"
    log "${YELLOW}⚠️  REVIEW THE SCRIPT BEFORE RUNNING IT!${NC}"
}

# =============================================================================
# MAIN EXECUTION
# =============================================================================

main() {
    log "${GREEN}🔍 REDUNDANCY DETECTION STARTED${NC}"
    log "📁 Project: ${PROJECT_ROOT}"
    
    detect_cmake_redundancy
    detect_main_redundancy  
    detect_build_redundancy
    detect_config_redundancy
    detect_header_redundancy
    detect_process_redundancy
    generate_cleanup_script
    
    section "SUMMARY"
    log "${GREEN}✅ Redundancy analysis complete!${NC}"
    log "📄 Log saved to: ${CLEANUP_LOG}"
    log "🧹 Review recommendations above and run cleanup script if needed"
    
    log "\n${YELLOW}QUICK WINS:${NC}"
    log "1. Remove old build/ directories"
    log "2. Use only the fixed platforms/pico_w/CMakeLists.txt"
    log "3. Use only the corrected platforms/pico_w/src/main.cpp"
    log "4. Consolidate board_config.h files"
    log "5. Clean up duplicate WebSocket/WiFi implementations"
}

main "$@"