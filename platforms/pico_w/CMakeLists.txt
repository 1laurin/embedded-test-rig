# Pico W CMakeLists.txt with proper WiFi and WebSocket configuration

cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Project name
project(diagnostic_rig_pico C CXX ASM)

# Set standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# =============================================================================
# CRITICAL: Memory management configuration for WiFi
# =============================================================================

# IMPORTANT: Set memory allocator BEFORE any WiFi libraries
# This fixes the MEM_LIBC_MALLOC incompatibility error
set(PICO_LWIP_CONFIG_FILE ${CMAKE_CURRENT_LIST_DIR}/lwipopts.h CACHE INTERNAL "")

# Add compile definitions for memory management
add_compile_definitions(
    # Memory management - CRITICAL for WiFi compatibility
    MEMP_NUM_TCP_PCB=10
    MEMP_NUM_TCP_PCB_LISTEN=8
    MEMP_NUM_UDP_PCB=4
    MEMP_NUM_SYS_TIMEOUT=10
    PBUF_POOL_SIZE=24
    TCP_MSS=1460
    TCP_WND=4*TCP_MSS
    TCP_SND_BUF=4*TCP_MSS
    
    # Disable incompatible memory allocator
    MEM_LIBC_MALLOC=0
    MEMP_MEM_MALLOC=0
    
    # Enable proper lwIP memory management
    MEM_ALIGNMENT=4
    MEM_SIZE=4000
)

# =============================================================================
# Source file collection
# =============================================================================

# Collect all source files
file(GLOB COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/system/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/system/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/monitoring/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/monitoring/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/demo/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/demo/*.c"
)

# Platform specific sources
file(GLOB PICO_PLATFORM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/*.c"
    # Add WebSocket integration sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wifi_manager.cpp"
)

# =============================================================================
# Include directories
# =============================================================================

set(INCLUDE_DIRS
    # Common includes
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/monitoring"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/demo"
    
    # Platform specific includes
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal"
)

# =============================================================================
# Create executable
# =============================================================================

add_executable(${PROJECT_NAME}
    ${COMMON_SOURCES}
    ${PICO_PLATFORM_SOURCES}
)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIRS}
)

# =============================================================================
# Link libraries (ORDER MATTERS for WiFi)
# =============================================================================

target_link_libraries(${PROJECT_NAME}
    # Core Pico libraries
    pico_stdlib
    
    # Hardware libraries
    hardware_gpio
    hardware_uart
    hardware_adc
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_timer
    hardware_irq
    hardware_clocks
    hardware_pio
    
    # WiFi and networking libraries (specific order required)
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip
    pico_lwip_http
)

# =============================================================================
# Compile definitions
# =============================================================================

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Board identification
    PICO_BOARD_NAME="pico_w"
    BOARD_NAME="Raspberry Pi Pico W"
    MCU_NAME="RP2040"
    MCU_FREQUENCY_HZ=125000000
    
    # Enable WiFi and networking features
    WIFI_ENABLED=1
    WEBSOCKET_ENABLED=1
    CYW43_USE_SPI=1
    
    # lwIP configuration
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
    CYW43_LWIP=1
    
    # Disable stdio USB for better stability (optional)
    # PICO_STDIO_USB=0
    # PICO_STDIO_UART=1
)

# =============================================================================
# Additional Pico SDK configurations
# =============================================================================

# Enable USB output, UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Set up compiler flags for optimization
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -O2
)

# =============================================================================
# Custom lwIP configuration (create lwipopts.h)
# =============================================================================

# Create a simple lwipopts.h if it doesn't exist
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/lwipopts.h")
    file(WRITE "${CMAKE_CURRENT_LIST_DIR}/lwipopts.h"
"#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Platform specific configuration for Pico W
#include \"lwipopts_examples_common.h\"

// Memory management - CRITICAL settings
#define MEM_LIBC_MALLOC 0
#define MEMP_MEM_MALLOC 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE 4000

// TCP/IP settings
#define MEMP_NUM_TCP_PCB 10
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_UDP_PCB 4
#define PBUF_POOL_SIZE 24
#define TCP_MSS 1460
#define TCP_WND (4 * TCP_MSS)
#define TCP_SND_BUF (4 * TCP_MSS)

// Enable necessary protocols
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DHCP 1
#define LWIP_DNS 1

// WebSocket support
#define LWIP_HTTPD 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI 1

#endif /* _LWIPOPTS_H */
")
endif()

message(STATUS "Configured for Pico W with WiFi and WebSocket support")
message(STATUS "Make sure websocket_server.cpp and wifi_manager.cpp exist in src/")