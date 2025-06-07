#!/bin/bash

# Project Structure Audit Script
# Creates a comprehensive report of all files and includes in the project

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
OUTPUT_FILE="$PROJECT_ROOT/project_audit.txt"

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE} Project Structure Audit Tool${NC}"
echo -e "${BLUE}==================================${NC}"
echo "Analyzing project at: $PROJECT_ROOT"
echo "Output file: $OUTPUT_FILE"
echo ""

# Start the audit report
cat > "$OUTPUT_FILE" << 'EOF'
# Multi-Channel Diagnostic Test Rig - Project Audit Report
Generated: $(date)
Project Root: $(pwd)

================================================================================
TABLE OF CONTENTS
================================================================================
1. DIRECTORY STRUCTURE
2. SOURCE FILES (.cpp, .c)
3. HEADER FILES (.h, .hpp)
4. INCLUDE ANALYSIS
5. MISSING FILES REPORT
6. CMAKE FILES
7. BUILD SCRIPTS
8. RECOMMENDATIONS

EOF

echo "$(date)" >> "$OUTPUT_FILE"
echo "Project Root: $PROJECT_ROOT" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Function to log to both console and file
log_both() {
    echo -e "$1"
    echo -e "$1" | sed 's/\x1b\[[0-9;]*m//g' >> "$OUTPUT_FILE"
}

# Function to scan directory structure
scan_directory_structure() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}1. DIRECTORY STRUCTURE${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Full directory tree:" >> "$OUTPUT_FILE"
    if command -v tree >/dev/null 2>&1; then
        tree -a -I '.git|build|bin|logs|.DS_Store' >> "$OUTPUT_FILE"
    else
        find . -type d -not -path './.git*' -not -path './build*' -not -path './bin*' | sort >> "$OUTPUT_FILE"
    fi
    echo "" >> "$OUTPUT_FILE"
}

# Function to scan source files
scan_source_files() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}2. SOURCE FILES (.cpp, .c)${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Source files by directory:" >> "$OUTPUT_FILE"
    
    # Find all source files
    find . -name "*.cpp" -o -name "*.c" | grep -v build | sort | while read file; do
        dir=$(dirname "$file")
        filename=$(basename "$file")
        size=$(wc -l < "$file" 2>/dev/null || echo "0")
        echo "  $file ($size lines)" >> "$OUTPUT_FILE"
        
        # Check if file is empty
        if [ "$size" -eq 0 ]; then
            log_both "${RED}    ⚠ EMPTY FILE: $file${NC}"
        fi
    done
    echo "" >> "$OUTPUT_FILE"
}

# Function to scan header files
scan_header_files() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}3. HEADER FILES (.h, .hpp)${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Header files by directory:" >> "$OUTPUT_FILE"
    
    # Find all header files
    find . -name "*.h" -o -name "*.hpp" | grep -v build | sort | while read file; do
        dir=$(dirname "$file")
        filename=$(basename "$file")
        size=$(wc -l < "$file" 2>/dev/null || echo "0")
        echo "  $file ($size lines)" >> "$OUTPUT_FILE"
        
        # Check if file is empty
        if [ "$size" -eq 0 ]; then
            log_both "${RED}    ⚠ EMPTY FILE: $file${NC}"
        fi
    done
    echo "" >> "$OUTPUT_FILE"
}

# Function to analyze includes
analyze_includes() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}4. INCLUDE ANALYSIS${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Include statements found in source files:" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # Find all includes in source files
    find . -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" | grep -v build | while read file; do
        if [ -f "$file" ] && [ -s "$file" ]; then
            includes=$(grep -n '#include' "$file" 2>/dev/null || true)
            if [ ! -z "$includes" ]; then
                echo "--- $file ---" >> "$OUTPUT_FILE"
                echo "$includes" >> "$OUTPUT_FILE"
                echo "" >> "$OUTPUT_FILE"
            fi
        fi
    done
}

# Function to find missing files
find_missing_files() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}5. MISSING FILES REPORT${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Checking for missing included files..." >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    missing_count=0
    
    # Extract all include statements and check if files exist
    find . -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" | grep -v build | while read source_file; do
        if [ -f "$source_file" ] && [ -s "$source_file" ]; then
            grep '#include' "$source_file" 2>/dev/null | while read include_line; do
                # Extract the included file name
                included_file=$(echo "$include_line" | sed 's/.*#include *["<]\([^">]*\)[">].*/\1/')
                
                if [[ "$included_file" != *"std"* && "$included_file" != *"pico/"* && "$included_file" != *"hardware/"* ]]; then
                    # Check various possible locations
                    found=false
                    
                    # Check relative to source file
                    source_dir=$(dirname "$source_file")
                    if [ -f "$source_dir/$included_file" ]; then
                        found=true
                    fi
                    
                    # Check in include directory
                    if [ -f "include/$included_file" ]; then
                        found=true
                    fi
                    
                    # Check in platforms/common
                    if [ -f "platforms/common/$included_file" ]; then
                        found=true
                    fi
                    
                    # Check in same directory as the file doing the including
                    base_name=$(basename "$included_file")
                    if find . -name "$base_name" -type f | grep -v build >/dev/null 2>&1; then
                        found=true
                    fi
                    
                    if [ "$found" = false ]; then
                        echo "❌ MISSING: $included_file (included by $source_file)" >> "$OUTPUT_FILE"
                        log_both "${RED}❌ MISSING: $included_file (included by $source_file)${NC}"
                        missing_count=$((missing_count + 1))
                    fi
                fi
            done
        fi
    done
    
    if [ "$missing_count" -eq 0 ]; then
        log_both "${GREEN}✅ No missing include files found${NC}"
    fi
    
    echo "" >> "$OUTPUT_FILE"
}

# Function to scan CMake files
scan_cmake_files() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}6. CMAKE FILES${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "CMake configuration files:" >> "$OUTPUT_FILE"
    
    find . -name "CMakeLists.txt" -o -name "*.cmake" | grep -v build | sort | while read file; do
        size=$(wc -l < "$file" 2>/dev/null || echo "0")
        echo "  $file ($size lines)" >> "$OUTPUT_FILE"
        
        if [ "$size" -eq 0 ]; then
            log_both "${RED}    ⚠ EMPTY CMAKE FILE: $file${NC}"
        fi
    done
    echo "" >> "$OUTPUT_FILE"
}

# Function to scan build scripts
scan_build_scripts() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}7. BUILD SCRIPTS${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Build and utility scripts:" >> "$OUTPUT_FILE"
    
    find . -name "*.sh" | grep -v build | sort | while read file; do
        size=$(wc -l < "$file" 2>/dev/null || echo "0")
        executable=""
        if [ -x "$file" ]; then
            executable=" (executable)"
        else
            executable=" (not executable)"
        fi
        echo "  $file ($size lines)$executable" >> "$OUTPUT_FILE"
        
        if [ "$size" -eq 0 ]; then
            log_both "${RED}    ⚠ EMPTY SCRIPT: $file${NC}"
        fi
    done
    echo "" >> "$OUTPUT_FILE"
}

# Function to generate recommendations
generate_recommendations() {
    log_both "${YELLOW}================================================================================${NC}"
    log_both "${YELLOW}8. RECOMMENDATIONS${NC}"
    log_both "${YELLOW}================================================================================${NC}"
    
    echo "Recommendations based on analysis:" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # Check for empty files
    empty_files=$(find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | grep -v build | xargs -I {} sh -c 'if [ ! -s "{}" ]; then echo "{}"; fi')
    if [ ! -z "$empty_files" ]; then
        echo "1. EMPTY FILES - These files need implementation:" >> "$OUTPUT_FILE"
        echo "$empty_files" | while read file; do
            echo "   - $file" >> "$OUTPUT_FILE"
        done
        echo "" >> "$OUTPUT_FILE"
        log_both "${YELLOW}📝 Found empty files that need implementation${NC}"
    fi
    
    # Check for inconsistent include paths
    echo "2. INCLUDE PATH CONSISTENCY:" >> "$OUTPUT_FILE"
    echo "   - Consider standardizing all includes to use relative paths from project root" >> "$OUTPUT_FILE"
    echo "   - Move all headers to include/ directory for consistency" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    # Check for platform-specific issues
    if [ -d "platforms/pico_w" ] && [ -d "platforms/nucleo_f446re" ]; then
        echo "3. MULTI-PLATFORM STRUCTURE:" >> "$OUTPUT_FILE"
        echo "   - Ensure HAL interfaces are consistent between platforms" >> "$OUTPUT_FILE"
        echo "   - Verify common code doesn't contain platform-specific includes" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
    fi
    
    echo "4. BUILD SYSTEM:" >> "$OUTPUT_FILE"
    echo "   - Verify CMakeLists.txt files have correct include paths" >> "$OUTPUT_FILE"
    echo "   - Ensure all source files are included in CMake GLOB patterns" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    
    log_both "${GREEN}✅ Audit complete! Check $OUTPUT_FILE for full details${NC}"
}

# Main execution
cd "$PROJECT_ROOT"

log_both "${BLUE}Starting comprehensive project audit...${NC}"
echo ""

scan_directory_structure
scan_source_files  
scan_header_files
analyze_includes
find_missing_files
scan_cmake_files
scan_build_scripts
generate_recommendations

log_both ""
log_both "${GREEN}=================================================================================${NC}"
log_both "${GREEN}AUDIT COMPLETE${NC}"
log_both "${GREEN}=================================================================================${NC}"
log_both "Full report saved to: $OUTPUT_FILE"
log_both ""
log_both "Quick summary:"
total_cpp=$(find . -name "*.cpp" | grep -v build | wc -l)
total_h=$(find . -name "*.h" | grep -v build | wc -l)
empty_files=$(find . -name "*.cpp" -o -name "*.c" -o -name "*.h" | grep -v build | xargs -I {} sh -c 'if [ ! -s "{}" ]; then echo "{}"; fi' | wc -l)

log_both "  📁 C++ source files: $total_cpp"
log_both "  📁 Header files: $total_h" 
log_both "  ⚠️  Empty files: $empty_files"

if [ "$empty_files" -gt 0 ]; then
    log_both ""
    log_both "${YELLOW}Next steps:${NC}"
    log_both "1. Review the missing files list in $OUTPUT_FILE"
    log_both "2. Create or move files to fix include issues"
    log_both "3. Implement empty files or remove unused includes"
fi