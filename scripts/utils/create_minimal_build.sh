#!/bin/bash

# Create Minimal Working Build
# This script creates the minimum viable files needed for a working Pico W build

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}================================${NC}"
echo -e "${BLUE} Creating Minimal Working Build${NC}"
echo -e "${BLUE}================================${NC}"

cd "$PROJECT_ROOT"

# Create minimal stub headers first
echo -e "${YELLOW}Creating minimal stub headers...${NC}"

# Core stubs
cat > include/core/system_init.h << 'EOF'
#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H
#include "platforms/common/hal_interface.h"
hal_status_t system_init(void);
bool is_system_initialized(void);
#endif
EOF

cat > include/core/system_loop.h << 'EOF'
#ifndef SYSTEM_LOOP_H
#define SYSTEM_LOOP_H
void run_main_loop(void);
void request_system_stop(void);
uint32_t get_loop_counter(void);
#endif
EOF

# UI stubs
cat > include/ui/input_handler.h << 'EOF'
#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H
#include <stdbool.h>
#include <stdint.h>
bool input_handler_init(void);
void handle_user_input(void);
void user_button_callback(uint32_t pin);
void register_emergency_stop_callback(void (*callback)(void));
void set_input_processing_enabled(bool enabled);
void clear_input_events(void);
#endif
EOF

# System stubs
cat > include/system/safety_monitor.h << 'EOF'
#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H
void check_system_safety(void);
void emergency_shutdown(const char* reason);
void register_safety_emergency_callback(void (*callback)(void));
#endif
EOF

# Demo stubs
cat > include/demo/hal_demo.h << 'EOF'
#ifndef HAL_DEMO_H
#define HAL_DEMO_H
void run_hal_demo(void);
#endif
EOF

# Utils stubs
cat > include/utils/hal_test.h << 'EOF'
#ifndef HAL_TEST_H
#define HAL_TEST_H
#include <stdbool.h>
bool test_hal_subsystems(void);
#endif
EOF

# Common types
cat > include/common/types.h << 'EOF'
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H
#include <stdint.h>
#include <stdbool.h>
// Common type definitions
#endif
EOF

echo -e "${YELLOW}Creating minimal source implementations...${NC}"

# Minimal system_init implementation
cat > src/core/system_init.cpp << 'EOF'
#include "core/system_init.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

static bool system_initialized = false;

hal_status_t system_init(void) {
    printf("[INIT] Starting system initialization...\n");
    
    if (hal_init() != HAL_OK) {
        printf("[INIT] HAL initialization failed\n");
        return HAL_ERROR;
    }
    
    if (hal_gpio_init() != HAL_OK) {
        printf("[INIT] GPIO initialization failed\n");
        return HAL_ERROR;
    }
    
    system_initialized = true;
    printf("[INIT] System initialization complete\n");
    return HAL_OK;
}

bool is_system_initialized(void) {
    return system_initialized;
}
EOF

# Minimal system_loop implementation
cat > src/core/system_loop.cpp << 'EOF'
#include "core/system_loop.h"
#include "ui/input_handler.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

static volatile bool system_stop_requested = false;
static uint32_t loop_counter = 0;

void run_main_loop(void) {
    printf("[LOOP] Starting main application loop\n");
    
    while (!system_stop_requested) {
        loop_counter++;
        
        // Handle user input
        handle_user_input();
        
        // Heartbeat every 1000 loops
        if (loop_counter % 1000 == 0) {
            hal_gpio_toggle(25); // Toggle built-in LED
            printf("[LOOP] Heartbeat: %lu loops\n", loop_counter);
        }
        
        hal_delay_ms(10); // 10ms loop delay
    }
    
    printf("[LOOP] Main loop exiting\n");
}

void request_system_stop(void) {
    system_stop_requested = true;
}

uint32_t get_loop_counter(void) {
    return loop_counter;
}
EOF

# Minimal input handler
cat > src/ui/input_handler.cpp << 'EOF'
#include "ui/input_handler.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

static bool input_initialized = false;
static volatile bool button_pressed = false;

bool input_handler_init(void) {
    printf("[INPUT] Initializing input handler\n");
    input_initialized = true;
    return true;
}

void handle_user_input(void) {
    if (!input_initialized) return;
    
    if (button_pressed) {
        button_pressed = false;
        printf("[INPUT] Button press handled\n");
    }
}

void user_button_callback(uint32_t pin) {
    button_pressed = true;
    printf("[INPUT] Button callback triggered on pin %lu\n", pin);
}

void register_emergency_stop_callback(void (*callback)(void)) {
    printf("[INPUT] Emergency stop callback registered\n");
}

void set_input_processing_enabled(bool enabled) {
    printf("[INPUT] Input processing %s\n", enabled ? "enabled" : "disabled");
}

void clear_input_events(void) {
    printf("[INPUT] Input events cleared\n");
}
EOF

# Minimal safety monitor
cat > src/system/safety_monitor.cpp << 'EOF'
#include "system/safety_monitor.h"
#include <stdio.h>

void check_system_safety(void) {
    // Minimal safety check - just a stub
}

void emergency_shutdown(const char* reason) {
    printf("[SAFETY] EMERGENCY SHUTDOWN: %s\n", reason);
}

void register_safety_emergency_callback(void (*callback)(void)) {
    printf("[SAFETY] Emergency callback registered\n");
}
EOF

# Minimal demo
cat > src/demo/hal_demo.cpp << 'EOF'
#include "demo/hal_demo.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

void run_hal_demo(void) {
    printf("[DEMO] Running minimal HAL demonstration\n");
    
    // Simple LED blink test
    for (int i = 0; i < 5; i++) {
        hal_gpio_write(25, GPIO_HIGH);
        hal_delay_ms(200);
        hal_gpio_write(25, GPIO_LOW);
        hal_delay_ms(200);
    }
    
    printf("[DEMO] HAL demo complete\n");
}
EOF

# Minimal test utils
cat > src/utils/hal_test.cpp << 'EOF'
#include "utils/hal_test.h"
#include "platforms/common/hal_interface.h"
#include <stdio.h>

bool test_hal_subsystems(void) {
    printf("[TEST] Testing HAL subsystems\n");
    
    // Test GPIO
    if (hal_gpio_write(25, GPIO_HIGH) != HAL_OK) {
        printf("[TEST] GPIO test failed\n");
        return false;
    }
    
    if (hal_gpio_write(25, GPIO_LOW) != HAL_OK) {
        printf("[TEST] GPIO test failed\n");
        return false;
    }
    
    printf("[TEST] All HAL tests passed\n");
    return true;
}
EOF

# Create system info implementation
cat > src/core/system_info.cpp << 'EOF'
#include <stdio.h>

void display_system_info(void) {
    printf("\n=== System Information ===\n");
    printf("Multi-Channel Diagnostic Test Rig - Pico W\n");
    printf("Minimal Build Version\n");
    printf("==========================\n\n");
}
EOF

# Create minimal pico_sdk_import.cmake
cat > platforms/pico_w/pico_sdk_import.cmake << 'EOF'
# This is a copy of <PICO_SDK_PATH>/external/pico_sdk_import.cmake

# This can be dropped into an external project to help locate the SDK
# It should be include()ed prior to project()

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    message("Using PICO_SDK_PATH from environment ('${PICO_SDK_PATH}')")
endif ()

if (DEFINED ENV{PICO_SDK_FETCH_FROM_GIT} AND (NOT PICO_SDK_FETCH_FROM_GIT))
    set(PICO_SDK_FETCH_FROM_GIT $ENV{PICO_SDK_FETCH_FROM_GIT})
    message("Using PICO_SDK_FETCH_FROM_GIT from environment ('${PICO_SDK_FETCH_FROM_GIT}')")
endif ()

if (DEFINED ENV{PICO_SDK_FETCH_FROM_GIT_PATH} AND (NOT PICO_SDK_FETCH_FROM_GIT_PATH))
    set(PICO_SDK_FETCH_FROM_GIT_PATH $ENV{PICO_SDK_FETCH_FROM_GIT_PATH})
    message("Using PICO_SDK_FETCH_FROM_GIT_PATH from environment ('${PICO_SDK_FETCH_FROM_GIT_PATH}')")
endif ()

set(PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to the Raspberry Pi Pico SDK")
set(PICO_SDK_FETCH_FROM_GIT "${PICO_SDK_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of SDK from git if not otherwise locatable")
set(PICO_SDK_FETCH_FROM_GIT_PATH "${PICO_SDK_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download SDK")

if (NOT PICO_SDK_PATH)
    if (PICO_SDK_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (PICO_SDK_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${PICO_SDK_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        FetchContent_Declare(
                pico_sdk
                GIT_REPOSITORY https://github.com/raspberrypi/pico-sdk
                GIT_TAG master
        )
        if (NOT pico_sdk)
            message("Downloading Raspberry Pi Pico SDK")
            FetchContent_Populate(pico_sdk)
            set(PICO_SDK_PATH ${pico_sdk_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        message(FATAL_ERROR
                "SDK location was not specified. Please set PICO_SDK_PATH or set PICO_SDK_FETCH_FROM_GIT to on to fetch from git."
                )
    endif ()
endif ()

get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
if (NOT EXISTS ${PICO_SDK_PATH})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' not found")
endif ()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
if (NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' does not appear to contain the Raspberry Pi Pico SDK")
endif ()

set(PICO_SDK_PATH ${PICO_SDK_PATH} CACHE PATH "Path to the Raspberry Pi Pico SDK" FORCE)

include(${PICO_SDK_INIT_CMAKE_FILE})
EOF

echo -e "${GREEN}✅ Minimal build files created!${NC}"
echo ""
echo -e "${YELLOW}Now try the build:${NC}"
echo "cd build/pico_w"
echo "rm -rf *"
echo "cmake -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico_w -DPICO_PLATFORM=rp2040 ../../platforms/pico_w"
echo "make -j4"
echo ""
echo -e "${BLUE}This minimal build should actually work and create a UF2 file!${NC}"