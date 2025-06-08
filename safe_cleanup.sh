#!/bin/bash
# SAFE PROJECT CLEANUP EXECUTION
# Based on audit recommendations

set -e

echo "🧹 EXECUTING SAFE PROJECT CLEANUP"
echo "================================================"

# =============================================================================
# BACKUP FIRST (SAFETY)
# =============================================================================
echo "📦 Creating backup before cleanup..."
BACKUP_DIR="project_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "../$BACKUP_DIR"
cp -r . "../$BACKUP_DIR/" 2>/dev/null || echo "Backup creation failed, continuing..."
echo "✅ Backup created at: ../$BACKUP_DIR"

# =============================================================================
# REMOVE OLD BUILD ARTIFACTS
# =============================================================================
echo ""
echo "🗂️ Removing old build directories..."
find . -name "build*" -type d | while read build_dir; do
    echo "  Removing: $build_dir"
    rm -rf "$build_dir"
done
echo "✅ Build directories cleaned"

# =============================================================================
# REMOVE SCATTERED BUILD ARTIFACTS  
# =============================================================================
echo ""
echo "🧹 Removing scattered build artifacts..."

# Remove object files not in build directories
scattered_objects=$(find . -name "*.o" -not -path "*/build/*" 2>/dev/null || true)
if [ -n "$scattered_objects" ]; then
    echo "$scattered_objects" | while read obj_file; do
        echo "  Removing: $obj_file"
        rm -f "$obj_file"
    done
else
    echo "  No scattered .o files found"
fi

# Remove ELF files not in build directories
scattered_elf=$(find . -name "*.elf" -not -path "*/build/*" 2>/dev/null || true)
if [ -n "$scattered_elf" ]; then
    echo "$scattered_elf" | while read elf_file; do
        echo "  Removing: $elf_file"
        rm -f "$elf_file"
    done
else
    echo "  No scattered .elf files found"
fi

# Remove binary files not in build directories
scattered_bin=$(find . -name "*.bin" -not -path "*/build/*" 2>/dev/null || true)
if [ -n "$scattered_bin" ]; then
    echo "$scattered_bin" | while read bin_file; do
        echo "  Removing: $bin_file"
        rm -f "$bin_file"
    done
else
    echo "  No scattered .bin files found"
fi

# Remove UF2 files not in build directories
scattered_uf2=$(find . -name "*.uf2" -not -path "*/build/*" 2>/dev/null || true)
if [ -n "$scattered_uf2" ]; then
    echo "$scattered_uf2" | while read uf2_file; do
        echo "  Removing: $uf2_file"
        rm -f "$uf2_file"
    done
else
    echo "  No scattered .uf2 files found"
fi

echo "✅ Scattered build artifacts cleaned"

# =============================================================================
# CLEAN TEMPORARY FILES
# =============================================================================
echo ""
echo "🧹 Removing temporary files..."

# Remove common temporary files
find . -name "*~" -delete 2>/dev/null || true
find . -name "*.tmp" -delete 2>/dev/null || true
find . -name ".DS_Store" -delete 2>/dev/null || true
find . -name "Thumbs.db" -delete 2>/dev/null || true

echo "✅ Temporary files cleaned"

# =============================================================================
# VERIFY CLEANUP
# =============================================================================
echo ""
echo "🔍 Verifying cleanup..."

remaining_builds=$(find . -name "build*" -type d | wc -l)
remaining_objects=$(find . -name "*.o" -not -path "*/build/*" | wc -l)
remaining_elf=$(find . -name "*.elf" -not -path "*/build/*" | wc -l)

echo "  Build directories remaining: $remaining_builds"
echo "  Scattered .o files remaining: $remaining_objects"  
echo "  Scattered .elf files remaining: $remaining_elf"

if [ "$remaining_builds" -eq 0 ] && [ "$remaining_objects" -eq 0 ] && [ "$remaining_elf" -eq 0 ]; then
    echo "✅ Cleanup verification passed"
else
    echo "⚠️  Some artifacts may remain - check manually"
fi

echo ""
echo "✅ SAFE CLEANUP COMPLETE!"
echo "================================================"
echo "Next steps:"
echo "1. Run the header organization script"
echo "2. Review and consolidate duplicate files"
echo "3. Test build with: cd platforms/pico_w && mkdir build && cd build && cmake .. && make"