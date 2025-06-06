#!/bin/bash

# Flash script for multi-channel diagnostic test rig
# Usage: ./flash.sh [platform]

set -e

PLATFORM=${1:-pico}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

case $PLATFORM in
    "pico"|"pico_w")
        echo "Flashing Raspberry Pi Pico W..."
        "$SCRIPT_DIR/scripts/deployment/flash_pico.sh"
        ;;
    "nucleo"|"stm32")
        echo "Flashing STM32 Nucleo-F446RE..."
        "$SCRIPT_DIR/scripts/deployment/flash_nucleo.sh"
        ;;
    *)
        echo "Usage: $0 [pico|nucleo]"
        echo "Supported platforms: pico, nucleo"
        exit 1
        ;;
esac

echo "Flashing completed successfully!"
