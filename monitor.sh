#!/bin/bash

# Serial monitor script for multi-channel diagnostic test rig
# Usage: ./monitor.sh [platform] [baud_rate]

set -e

PLATFORM=${1:-pico}
BAUD_RATE=${2:-115200}

case $PLATFORM in
    "pico"|"pico_w")
        echo "Starting serial monitor for Pico W (${BAUD_RATE} baud)..."
        # Auto-detect USB serial port for Pico
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            PORT=$(ls /dev/ttyACM* 2>/dev/null | head -n1)
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            PORT=$(ls /dev/tty.usbmodem* 2>/dev/null | head -n1)
        else
            echo "Please specify COM port manually"
            exit 1
        fi
        ;;
    "nucleo"|"stm32")
        echo "Starting serial monitor for Nucleo (${BAUD_RATE} baud)..."
        # Auto-detect USB serial port for Nucleo
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            PORT=$(ls /dev/ttyACM* 2>/dev/null | head -n1)
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            PORT=$(ls /dev/tty.usbmodem* 2>/dev/null | head -n1)
        else
            echo "Please specify COM port manually"
            exit 1
        fi
        ;;
    *)
        echo "Usage: $0 [pico|nucleo] [baud_rate]"
        exit 1
        ;;
esac

if [ -z "$PORT" ]; then
    echo "No serial port detected. Make sure device is connected."
    exit 1
fi

echo "Connecting to $PORT at $BAUD_RATE baud..."
echo "Press Ctrl+C to exit"

# Use appropriate serial monitor
if command -v screen >/dev/null 2>&1; then
    screen "$PORT" "$BAUD_RATE"
elif command -v minicom >/dev/null 2>&1; then
    minicom -D "$PORT" -b "$BAUD_RATE"
else
    echo "Please install 'screen' or 'minicom' for serial monitoring"
    exit 1
fi
