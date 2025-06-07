#!/bin/bash

# Pico W flash script for multi-channel diagnostic test rig
# Usage: ./flash_pico.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$PROJECT_ROOT/bin/pico_w"
BUILD_DIR="$PROJECT_ROOT/build/pico_w"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================${NC}"
echo -e "${BLUE} Flashing Raspberry Pi Pico W${NC}"
echo -e "${BLUE}================================${NC}"

# Look for UF2 file in both locations
UF2_FILES=(
    "$BIN_DIR/diagnostic_rig_pico.uf2"
    "$BUILD_DIR/diagnostic_rig_pico.uf2"
)

UF2_FILE=""
for file in "${UF2_FILES[@]}"; do
    if [ -f "$file" ]; then
        UF2_FILE="$file"
        break
    fi
done

if [ -z "$UF2_FILE" ]; then
    echo -e "${RED}ERROR: No UF2 file found${NC}"
    echo "Checked locations:"
    for file in "${UF2_FILES[@]}"; do
        echo "  $file"
    done
    echo ""
    echo "Please build the project first:"
    echo "  ./build.sh pico"
    exit 1
fi

echo -e "${BLUE}Found UF2 file: $UF2_FILE${NC}"
ls -la "$UF2_FILE"

# Check if Pico is in BOOTSEL mode
RPI_MOUNT="/Volumes/RPI-RP2"
if [ ! -d "$RPI_MOUNT" ]; then
    echo -e "${YELLOW}Pico not in BOOTSEL mode${NC}"
    echo ""
    echo "Please put Pico in BOOTSEL mode:"
    echo "1. Disconnect Pico from USB"
    echo "2. Hold down the BOOTSEL button"
    echo "3. Connect USB cable while holding BOOTSEL"
    echo "4. Release BOOTSEL button"
    echo "5. RPI-RP2 drive should appear in Finder"
    echo ""
    echo "Waiting for RPI-RP2 drive..."
    
    # Wait for RPI-RP2 to appear (up to 30 seconds)
    for i in {1..30}; do
        if [ -d "$RPI_MOUNT" ]; then
            echo -e "${GREEN}✓ RPI-RP2 drive detected!${NC}"
            break
        fi
        echo -n "."
        sleep 1
    done
    echo ""
    
    if [ ! -d "$RPI_MOUNT" ]; then
        echo -e "${RED}ERROR: RPI-RP2 drive not found${NC}"
        echo "Please manually put Pico in BOOTSEL mode and try again"
        exit 1
    fi
fi

# Flash the UF2 file
echo -e "${BLUE}Flashing firmware...${NC}"
echo "Copying $UF2_FILE to $RPI_MOUNT"

cp "$UF2_FILE" "$RPI_MOUNT/"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Firmware copied successfully${NC}"
    echo -e "${BLUE}Pico will automatically reboot and disconnect...${NC}"
    
    # Wait a moment for the copy to complete
    sleep 2
    
    # Check if RPI-RP2 drive is gone (indicates successful flash)
    if [ ! -d "$RPI_MOUNT" ]; then
        echo -e "${GREEN}✓ Pico rebooted - Flash successful!${NC}"
    else
        echo -e "${YELLOW}⚠ RPI-RP2 drive still present - Flash may have failed${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}Flashing completed!${NC}"
    echo "Disconnect and reconnect USB to start normal operation"
    echo "Use './monitor.sh pico' to view serial output"
    
else
    echo -e "${RED}ERROR: Failed to copy firmware${NC}"
    exit 1
fi

echo -e "${BLUE}================================${NC}"