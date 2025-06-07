#!/bin/bash

# Debug connection script for multi-channel diagnostic test rig
# Helps troubleshoot USB/Serial connection issues

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================${NC}"
echo -e "${BLUE} Connection Debug Tool${NC}"
echo -e "${BLUE}================================${NC}"
echo ""

# Check operating system
echo -e "${YELLOW}System Information:${NC}"
echo "OS: $(uname -s)"
echo "Architecture: $(uname -m)"
echo "Date: $(date)"
echo ""

# Check USB devices
echo -e "${YELLOW}USB Devices:${NC}"
if command -v system_profiler >/dev/null 2>&1; then
    # macOS
    echo "Raspberry Pi devices:"
    system_profiler SPUSBDataType 2>/dev/null | grep -A 5 -B 2 -i "raspberry\|pico" || echo "No Raspberry Pi devices found"
    echo ""
    echo "STM devices:"
    system_profiler SPUSBDataType 2>/dev/null | grep -A 5 -B 2 -i "stm\|nucleo" || echo "No STM devices found"
elif command -v lsusb >/dev/null 2>&1; then
    # Linux
    echo "All USB devices:"
    lsusb
    echo ""
    echo "Raspberry Pi devices:"
    lsusb | grep -i "raspberry\|pico" || echo "No Raspberry Pi devices found"
    echo ""
    echo "STM devices:"
    lsusb | grep -i "stm\|nucleo" || echo "No STM devices found"
else
    echo "Unable to detect USB devices (no system_profiler or lsusb)"
fi
echo ""

# Check serial devices
echo -e "${YELLOW}Serial Devices:${NC}"
echo "USB Modem devices:"
ls -la /dev/tty.usbmodem* 2>/dev/null || echo "  None found"
ls -la /dev/cu.usbmodem* 2>/dev/null || echo "  None found"

echo ""
echo "ACM devices (Linux/some macOS):"
ls -la /dev/ttyACM* 2>/dev/null || echo "  None found"

echo ""
echo "All TTY devices (first 20):"
ls -la /dev/tty.* 2>/dev/null | head -20

echo ""

# Check permissions
echo -e "${YELLOW}Permission Check:${NC}"
FOUND_DEVICES=($(ls /dev/tty.usbmodem* /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null))

if [ ${#FOUND_DEVICES[@]} -gt 0 ]; then
    for device in "${FOUND_DEVICES[@]}"; do
        if [ -e "$device" ]; then
            echo "Device: $device"
            ls -la "$device"
            if [ -r "$device" ] && [ -w "$device" ]; then
                echo -e "  ${GREEN}✓ Read/Write permissions OK${NC}"
            else
                echo -e "  ${RED}✗ Insufficient permissions${NC}"
                echo -e "  ${YELLOW}Fix with: sudo chmod 666 $device${NC}"
            fi
            echo ""
        fi
    done
else
    echo "No potential serial devices found"
fi

# Check for serial monitor tools
echo -e "${YELLOW}Serial Monitor Tools:${NC}"
TOOLS=("screen" "minicom" "cu" "picocom")
for tool in "${TOOLS[@]}"; do
    if command -v "$tool" >/dev/null 2>&1; then
        echo -e "  ${GREEN}✓ $tool${NC} - $(which $tool)"
    else
        echo -e "  ${RED}✗ $tool${NC} - Not installed"
    fi
done

echo ""

# Installation suggestions
echo -e "${YELLOW}Installation Suggestions:${NC}"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "macOS:"
    echo "  brew install screen"
    echo "  brew install minicom"
elif [[ "$OSTYPE" == "linux"* ]]; then
    echo "Ubuntu/Debian:"
    echo "  sudo apt update"
    echo "  sudo apt install screen minicom"
    echo ""
    echo "Add user to dialout group:"
    echo "  sudo usermod -a -G dialout \$USER"
    echo "  (then logout/login)"
fi

echo ""

# Pico-specific troubleshooting
echo -e "${YELLOW}Pico W Troubleshooting:${NC}"
echo "1. Make sure Pico W is connected via USB"
echo "2. Pico should NOT be in BOOTSEL mode (don't hold BOOTSEL button)"
echo "3. Green LED should be on (power)"
echo "4. If flashing failed, try holding BOOTSEL while connecting USB"
echo "5. After successful flash, disconnect and reconnect USB"
echo ""

# Connection test
echo -e "${YELLOW}Connection Test:${NC}"
if [ ${#FOUND_DEVICES[@]} -gt 0 ]; then
    echo "Found potential devices. Testing connection..."
    for device in "${FOUND_DEVICES[@]}"; do
        if [ -e "$device" ]; then
            echo "Testing $device..."
            
            # Try to open the device briefly
            if timeout 2s cat "$device" >/dev/null 2>&1; then
                echo -e "  ${GREEN}✓ Device responds${NC}"
            else
                echo -e "  ${YELLOW}? Device exists but no immediate response${NC}"
                echo "    (This is normal if device isn't sending data)"
            fi
        fi
    done
else
    echo -e "${RED}No devices to test${NC}"
fi

echo ""
echo -e "${BLUE}================================${NC}"
echo -e "${BLUE} Debug Complete${NC}"
echo -e "${BLUE}================================${NC}"

# Suggest next steps
echo ""
echo -e "${YELLOW}Next Steps:${NC}"
echo "1. If devices found: ./monitor.sh pico 115200 /dev/tty.usbmodem[TAB]"
echo "2. If no devices: Check physical connection and try re-flashing"
echo "3. If permission issues: sudo chmod 666 /dev/tty.usbmodem*"
echo "4. If still issues: Try different USB cable or port"