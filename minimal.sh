#!/bin/bash

# Minimal Test Build Script
# This script creates the absolute minimum files needed to test the build system fixes

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Creating Minimal Test Build ===${NC}"

# Create missing header that's preventing builds
mkdir -p "$PROJECT_ROOT/include/monitoring"
if [ ! -f "$PROJECT_ROOT/include/monitoring/diagnostics_engine.h" ]; then
    cat > "$PROJECT_ROOT/include/monitoring/diagnostics_engine.h" << 'EOF'
#ifndef DIAGNOSTICS_ENGINE_H
#define DIAGNOSTICS_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_DIAGNOSTIC_CHANNELS 4

// Minimal function declarations to satisfy includes
void test_diagnostic_channels(void);
void toggle_all_channels(void);
void get_channel_states(bool* states);
void set_channel_enable(int channel, bool enable);
void print_system_status(void);
void update_display_status(void);

#ifdef __cplusplus
}
#endif

#endif // DIAGNOSTICS_ENGINE_H
EOF
    echo -e "${GREEN}✓ Created diagnostics_engine.h${NC}"
fi

# Create the missing hal_main.cpp that should contain the main HAL functions
mkdir -p "$PROJECT_ROOT/platforms/pico_w/hal"
if [ ! -f "$PROJECT_ROOT/platforms/pico_w/hal/hal_main.cpp" ] || [ ! -s "$PROJECT_ROOT/platforms/pico_w/hal/hal_main.cpp" ]; then
    cat > "$PROJECT_ROOT/platforms/pico_w/hal/hal_main.cpp" << 'EOF'
/**
 * @file hal_main.cpp
 * @brief Main HAL initialization and system functions for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include <stdio.h>

static bool hal_system_initialized = false;

hal_status_t hal_init(void) {
    if (hal_system_initialized) {
        return HAL_OK;
    }

    printf("[HAL] Initializing Pico W HAL layer...\n");
    
    // Initialize standard I/O and basic clocks
    stdio_init_all();
    
    hal_system_initialized = true;
    printf("[HAL] HAL layer initialized successfully\n");
    
    return HAL_OK;
}

hal_status_t hal_deinit(void) {
    if (!hal_system_initialized) {
        return HAL_OK;
    }
    
    printf("[HAL] Deinitializing HAL layer...\n");
    hal_system_initialized = false;
    
    return HAL_OK;
}

uint32_t hal_get_tick_ms(void) {
    return to_ms_since_boot(get_absolute_time());
}

void hal_delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

void hal_delay_us(uint32_t us) {
    sleep_us(us);
}

void hal_system_reset(void) {
    watchdog_enable(1, 1);
    while(1);
}
EOF
    echo -e "${GREEN}✓ Created hal_main.cpp${NC}"
fi

# Create missing ADC HAL implementation
if [ ! -f "$PROJECT_ROOT/platforms/pico_w/hal/adc_hal.cpp" ] || [ ! -s "$PROJECT_ROOT/platforms/pico_w/hal/adc_hal.cpp" ]; then
    cat > "$PROJECT_ROOT/platforms/pico_w/hal/adc_hal.cpp" << 'EOF'
/**
 * @file adc_hal.cpp
 * @brief ADC Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

static bool adc_initialized = false;

hal_status_t hal_adc_init(void) {
    if (adc_initialized) {
        return HAL_OK;
    }
    
    printf("[ADC] Initializing ADC subsystem...\n");
    
    adc_init();
    
    // Enable temperature sensor
    adc_set_temp_sensor_enabled(true);
    
    adc_initialized = true;
    printf("[ADC] ADC subsystem initialized\n");
    
    return HAL_OK;
}

hal_status_t hal_adc_config(const adc_config_t *config) {
    if (!adc_initialized || !config) {
        return HAL_ERROR;
    }
    
    if (config->channel < 5) {  // Pico has ADC channels 0-4
        if (config->channel < 3) {
            // Enable GPIO for ADC (GPIO 26, 27, 28 = ADC 0, 1, 2)
            adc_gpio_init(26 + config->channel);
        }
        return HAL_OK;
    }
    
    return HAL_INVALID_PARAM;
}

hal_status_t hal_adc_read(uint8_t channel, uint16_t *value) {
    if (!adc_initialized || !value || channel >= 5) {
        return HAL_INVALID_PARAM;
    }
    
    adc_select_input(channel);
    *value = adc_read();
    
    return HAL_OK;
}

hal_status_t hal_adc_read_voltage(uint8_t channel, float *voltage) {
    if (!voltage) {
        return HAL_INVALID_PARAM;
    }
    
    uint16_t raw_value;
    hal_status_t status = hal_adc_read(channel, &raw_value);
    
    if (status == HAL_OK) {
        *voltage = (float)raw_value * ADC_REFERENCE_VOLTAGE / (1 << ADC_RESOLUTION_BITS);
    }
    
    return status;
}

hal_status_t hal_adc_start_continuous(uint8_t channel, void (*callback)(uint8_t, uint16_t)) {
    // Stub implementation
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_adc_stop_continuous(uint8_t channel) {
    // Stub implementation
    return HAL_NOT_SUPPORTED;
}
EOF
    echo -e "${GREEN}✓ Created adc_hal.cpp${NC}"
fi

# Create system_info implementation
mkdir -p "$PROJECT_ROOT/src/core"
if [ ! -f "$PROJECT_ROOT/src/core/system_info.cpp" ] || [ ! -s "$PROJECT_ROOT/src/core/system_info.cpp" ]; then
    cat > "$PROJECT_ROOT/src/core/system_info.cpp" << 'EOF'
/**
 * @file system_info.cpp
 * @brief System information display
 */

#include <stdio.h>

void display_system_info(void) {
    printf("\n=== Multi-Channel Diagnostic Test Rig ===\n");
    printf("Platform: Raspberry Pi Pico W\n");
    printf("Version: 1.0.0\n");
    printf("Build: Minimal Test Build\n");
    printf("==========================================\n\n");
}
EOF
    echo -e "${GREEN}✓ Created system_info.cpp${NC}"
fi

# Create minimal diagnostics_engine implementation
mkdir -p "$PROJECT_ROOT/src/monitoring"
if [ ! -f "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" ] || [ ! -s "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" ]; then
    cat > "$PROJECT_ROOT/src/monitoring/diagnostics_engine.cpp" << 'EOF'
/**
 * @file diagnostics_engine.cpp
 * @brief Minimal diagnostics engine implementation
 */

#include "monitoring/diagnostics_engine.h"
#include <stdio.h>

void test_diagnostic_channels(void) {
    printf("[DIAG] Testing diagnostic channels...\n");
}

void toggle_all_channels(void) {
    printf("[DIAG] Toggling all channels\n");
}

void get_channel_states(bool* states) {
    if (states) {
        for (int i = 0; i < NUM_DIAGNOSTIC_CHANNELS; i++) {
            states[i] = true; // All enabled for demo
        }
    }
}

void set_channel_enable(int channel, bool enable) {
    printf("[DIAG] Channel %d %s\n", channel, enable ? "enabled" : "disabled");
}

void print_system_status(void) {
    printf("[DIAG] System Status: Running\n");
}

void update_display_status(void) {
    // Minimal implementation - just a stub
}
EOF
    echo -e "${GREEN}✓ Created diagnostics_engine.cpp${NC}"
fi

# Fix the corrected CMakeLists.txt in place
echo -e "${YELLOW}Updating Pico W CMakeLists.txt...${NC}"
cp "$PROJECT_ROOT/platforms/pico_w/CMakeLists.txt" "$PROJECT_ROOT/platforms/pico_w/CMakeLists.txt.backup" 2>/dev/null || true

# Apply the fixed CMakeLists.txt directly
cat > "$PROJECT_ROOT/platforms/pico_w/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK BEFORE project() call
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(diagnostic_rig_pico VERSION 1.0.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

# CRITICAL: Fix for macOS cross-compilation linker issues
if(APPLE)
    set(CMAKE_C_LINK_FLAGS "")
    set(CMAKE_CXX_LINK_FLAGS "")
    unset(CMAKE_OSX_DEPLOYMENT_TARGET)
endif()

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/core
    ${PROJECT_ROOT}/include/ui
    ${PROJECT_ROOT}/include/system
    ${PROJECT_ROOT}/include/demo
    ${PROJECT_ROOT}/include/utils
    ${PROJECT_ROOT}/include/monitoring
    ${PROJECT_ROOT}/include/common
    ${PROJECT_ROOT}/platforms/common
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Collect sources safely
function(safe_add_sources PATTERN RESULT_VAR)
    file(GLOB FOUND_SOURCES ${PATTERN})
    foreach(SRC ${FOUND_SOURCES})
        if(EXISTS ${SRC})
            file(SIZE ${SRC} SRC_SIZE)
            if(SRC_SIZE GREATER 0)
                list(APPEND ${RESULT_VAR} ${SRC})
                set(${RESULT_VAR} ${${RESULT_VAR}} PARENT_SCOPE)
            endif()
        endif()
    endforeach()
endfunction()

set(PICO_PLATFORM_SOURCES "")
safe_add_sources("${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" PICO_PLATFORM_SOURCES)
safe_add_sources("${CMAKE_CURRENT_SOURCE_DIR}/hal/*.cpp" PICO_PLATFORM_SOURCES)

set(COMMON_SOURCES "")
set(COMMON_SOURCE_DIRS
    "${PROJECT_ROOT}/src/core"
    "${PROJECT_ROOT}/src/ui"
    "${PROJECT_ROOT}/src/system"
    "${PROJECT_ROOT}/src/demo"
    "${PROJECT_ROOT}/src/utils"
    "${PROJECT_ROOT}/src/monitoring"
)

foreach(DIR ${COMMON_SOURCE_DIRS})
    if(EXISTS ${DIR})
        safe_add_sources("${DIR}/*.cpp" COMMON_SOURCES)
    endif()
endforeach()

add_executable(${PROJECT_NAME}
    ${PICO_PLATFORM_SOURCES}
    ${COMMON_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_BOARD_NAME="pico_w"
    PLATFORM_PICO_W=1
    NUM_DIAGNOSTIC_CHANNELS=4
    MAIN_LOOP_DELAY_MS=100
    HEARTBEAT_INTERVAL_MS=1000
    STATUS_UPDATE_INTERVAL_MS=5000
    SAFETY_CHECK_INTERVAL_MS=500
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_uart
    hardware_adc
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_timer
    hardware_irq
    hardware_clocks
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "diagnostic_rig_pico"
)

list(LENGTH PICO_PLATFORM_SOURCES PICO_SRC_COUNT)
list(LENGTH COMMON_SOURCES COMMON_SRC_COUNT)
math(EXPR TOTAL_SRC_COUNT "${PICO_SRC_COUNT} + ${COMMON_SRC_COUNT}")

message(STATUS "=== Pico W Build Configuration ===")
message(STATUS "Platform Sources: ${PICO_SRC_COUNT}")
message(STATUS "Common Sources: ${COMMON_SRC_COUNT}")
message(STATUS "Total Sources: ${TOTAL_SRC_COUNT}")
message(STATUS "================================")
EOF

echo -e "${GREEN}✓ Updated CMakeLists.txt${NC}"

# Test the build
echo ""
echo -e "${BLUE}Testing the build...${NC}"

if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${RED}ERROR: PICO_SDK_PATH not set${NC}"
    echo "Please run: export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

if [ ! -d "$PICO_SDK_PATH" ]; then
    echo -e "${RED}ERROR: PICO_SDK_PATH does not exist: $PICO_SDK_PATH${NC}"
    exit 1
fi

# Clean and create build directory
BUILD_DIR="$PROJECT_ROOT/build/pico_w_minimal"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${YELLOW}Running CMake configure...${NC}"

if cmake -DCMAKE_BUILD_TYPE=Release \
         -DPICO_BOARD=pico_w \
         -DPICO_PLATFORM=rp2040 \
         ../../platforms/pico_w; then
    echo -e "${GREEN}✓ CMake configure successful${NC}"
    
    echo -e "${YELLOW}Building...${NC}"
    if make -j4; then
        echo -e "${GREEN}✅ BUILD SUCCESSFUL!${NC}"
        echo ""
        echo -e "${BLUE}Build outputs:${NC}"
        ls -la diagnostic_rig_pico.*
        echo ""
        echo -e "${GREEN}Your UF2 file is ready: $(pwd)/diagnostic_rig_pico.uf2${NC}"
        echo ""
        echo -e "${YELLOW}To flash:${NC}"
        echo "1. Hold BOOTSEL button on Pico W"
        echo "2. Connect USB cable"
        echo "3. Release BOOTSEL button"
        echo "4. Copy the UF2 file to the RPI-RP2 drive that appears"
        echo ""
        echo "Or use your existing flash script:"
        echo "  ./flash.sh pico"
    else
        echo -e "${RED}✗ Build failed${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ CMake configure failed${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}=== Minimal Test Build Complete ===${NC}"
echo "The build system fixes have been applied and tested successfully!"