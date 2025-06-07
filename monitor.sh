#!/bin/bash

# Improved Serial monitor script for multi-channel diagnostic test rig
# Usage: ./monitor.sh [platform] [baud_rate] [port]

set -e

PLATFORM=${1:-pico}
BAUD_RATE=${2:-115200}
MANUAL_PORT=$3

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Serial Monitor for ${PLATFORM} ===${NC}"

# Function to detect Pico W serial port
detect_pico_port() {
    # Try multiple patterns for Pico detection
    local pico_ports=(
        $(ls /dev/tty.usbmodem* 2>/dev/null | head -1)
        $(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
        $(ls /dev/ttyACM* 2>/dev/null | head -1)
    )
    
    for port in "${pico_ports[@]}"; do
        if [ -e "$port" ]; then
            echo "$port"
            return 0
        fi
    done
    
    return 1
}

# Function to detect STM32 Nucleo port
detect_nucleo_port() {
    local nucleo_ports=(
        $(ls /dev/tty.usbmodem* 2>/dev/null | grep -E "(STM|NUCLEO)" | head -1)
        $(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
        $(ls /dev/ttyACM* 2>/dev/null | head -1)
    )
    
    for port in "${nucleo_ports[@]}"; do
        if [ -e "$port" ]; then
            echo "$port"
            return 0
        fi
    done
    
    return 1
}

# Function to list all available serial ports
list_serial_ports() {
    echo -e "${YELLOW}Available serial ports:${NC}"
    echo "USB Modem devices:"
    ls -la /dev/tty.usbmodem* 2>/dev/null || echo "  None found"
    ls -la /dev/cu.usbmodem* 2>/dev/null || echo "  None found"
    
    echo "ACM devices (Linux):"
    ls -la /dev/ttyACM* 2>/dev/null || echo "  None found"
    
    echo "All tty devices:"
    ls -la /dev/tty.* 2>/dev/null | head -10
}

# Determine the correct port
PORT=""

if [ -n "$MANUAL_PORT" ]; then
    echo -e "${BLUE}Using manually specified port: $MANUAL_PORT${NC}"
    PORT="$MANUAL_PORT"
elif [ "$PLATFORM" = "pico" ] || [ "$PLATFORM" = "pico_w" ]; then
    echo -e "${BLUE}Detecting Pico W serial port...${NC}"
    PORT=$(detect_pico_port)
elif [ "$PLATFORM" = "nucleo" ] || [ "$PLATFORM" = "stm32" ]; then
    echo -e "${BLUE}Detecting Nucleo serial port...${NC}"
    PORT=$(detect_nucleo_port)
else
    echo -e "${RED}Unknown platform: $PLATFORM${NC}"
    echo "Usage: $0 [pico|nucleo] [baud_rate] [manual_port]"
    exit 1
fi

# Check if port was found
if [ -z "$PORT" ] || [ ! -e "$PORT" ]; then
    echo -e "${RED}No serial port detected for $PLATFORM${NC}"
    echo ""
    list_serial_ports
    echo ""
    echo -e "${YELLOW}Troubleshooting steps:${NC}"
    echo "1. Make sure the device is connected via USB"
    echo "2. Check that the device is powered on"
    echo "3. Try a different USB cable"
    echo "4. For Pico W: Make sure it's in normal mode (not BOOTSEL)"
    echo "5. Check USB permissions: sudo chmod 666 /dev/tty*"
    echo ""
    echo "Manual usage: $0 $PLATFORM $BAUD_RATE /dev/tty.usbmodem14101"
    exit 1
fi

echo -e "${GREEN}Found serial port: $PORT${NC}"
echo -e "${BLUE}Connecting at $BAUD_RATE baud...${NC}"
echo -e "${YELLOW}Press Ctrl+C to exit${NC}"
echo ""

# Check if port is accessible
if [ ! -r "$PORT" ] || [ ! -w "$PORT" ]; then
    echo -e "${YELLOW}Warning: Port permissions may be insufficient${NC}"
    echo "Try running: sudo chmod 666 $PORT"
    echo ""
fi

# Try different serial monitor tools
if command -v screen >/dev/null 2>&1; then
    echo -e "${BLUE}Using 'screen' for serial monitoring${NC}"
    echo "Commands: Ctrl+A then K to quit, Ctrl+A then D to detach"
    sleep 2
    screen "$PORT" "$BAUD_RATE"
elif command -v minicom >/dev/null 2>&1; then
    echo -e "${BLUE}Using 'minicom' for serial monitoring${NC}"
    echo "Commands: Ctrl+A then X to quit"
    sleep 2
    minicom -D "$PORT" -b "$BAUD_RATE"
elif command -v cu >/dev/null 2>&1; then
    echo -e "${BLUE}Using 'cu' for serial monitoring${NC}"
    echo "Commands: Type ~. to quit"
    sleep 2
    cu -l "$PORT" -s "$BAUD_RATE"
else
    echo -e "${RED}No serial monitor found!${NC}"
    echo "Please install one of: screen, minicom, or cu"
    echo ""
    echo "macOS: brew install screen"
    echo "Ubuntu: sudo apt install screen minicom"
    exit 1
fi