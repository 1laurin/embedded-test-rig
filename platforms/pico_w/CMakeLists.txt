cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set project name and version
project(diagnostic_rig_pico VERSION 1.0.0 LANGUAGES C CXX ASM)

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Initialize the Pico SDK
pico_sdk_init()

# Add compiler options
add_compile_options(-Wall -Wextra)

# Define project root for easier path management
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# =============================================================================
# INCLUDE DIRECTORIES - COMPREHENSIVE LIST
# =============================================================================

# Add all include directories that the project needs
set(INCLUDE_DIRS
    # Project root includes
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/core
    ${PROJECT_ROOT}/include/ui
    ${PROJECT_ROOT}/include/system
    ${PROJECT_ROOT}/include/demo
    ${PROJECT_ROOT}/include/utils
    ${PROJECT_ROOT}/include/common
    ${PROJECT_ROOT}/include/drivers
    ${PROJECT_ROOT}/include/logging
    ${PROJECT_ROOT}/include/monitoring
    ${PROJECT_ROOT}/include/protocols
    
    # Platform common
    ${PROJECT_ROOT}/platforms/common
    
    # Platform specific
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    
    # Source directories (for headers next to sources)
    ${PROJECT_ROOT}/src
    ${PROJECT_ROOT}/src/core
    ${PROJECT_ROOT}/src/ui
    ${PROJECT_ROOT}/src/system
    ${PROJECT_ROOT}/src/demo
    ${PROJECT_ROOT}/src/utils
    ${PROJECT_ROOT}/src/monitoring
)

# Apply include directories globally
include_directories(${INCLUDE_DIRS})

# =============================================================================
# SOURCE FILE COLLECTION
# =============================================================================

# Platform-specific Pico W sources
file(GLOB PICO_PLATFORM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hal/*.c"
)

# Common source files - only include non-empty files
set(COMMON_SOURCE_DIRS
    "${PROJECT_ROOT}/src/core"
    "${PROJECT_ROOT}/src/ui"
    "${PROJECT_ROOT}/src/system"
    "${PROJECT_ROOT}/src/demo"
    "${PROJECT_ROOT}/src/utils"
    "${PROJECT_ROOT}/src/monitoring"
)

set(COMMON_SOURCES "")
foreach(DIR ${COMMON_SOURCE_DIRS})
    if(EXISTS ${DIR})
        file(GLOB DIR_SOURCES "${DIR}/*.cpp" "${DIR}/*.c")
        foreach(SRC ${DIR_SOURCES})
            # Only include files that are not empty (have size > 0)
            file(SIZE ${SRC} SRC_SIZE)
            if(SRC_SIZE GREATER 0)
                list(APPEND COMMON_SOURCES ${SRC})
            endif()
        endforeach()
    endif()
endforeach()

# =============================================================================
# EXECUTABLE CREATION
# =============================================================================

# Create the main executable
add_executable(${PROJECT_NAME}
    ${PICO_PLATFORM_SOURCES}
    ${COMMON_SOURCES}
)

# =============================================================================
# TARGET CONFIGURATION
# =============================================================================

# Set target-specific include directories (in addition to global ones)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIRS}
)

# Set target properties
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Add target compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_BOARD_NAME="pico_w"
    BOARD_NAME="Raspberry Pi Pico W"
    MCU_NAME="RP2040"
    MCU_FREQUENCY_HZ=125000000
)

# =============================================================================
# LIBRARY LINKING
# =============================================================================

# Link Pico SDK libraries
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
    hardware_pio
)

# Conditionally add WiFi libraries if needed
option(ENABLE_WIFI "Enable WiFi support" OFF)
if(ENABLE_WIFI)
    target_link_libraries(${PROJECT_NAME}
        pico_cyw43_arch_lwip_threadsafe_background
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIFI_ENABLED=1
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIFI_ENABLED=0
    )
endif()

# =============================================================================
# STDIO CONFIGURATION
# =============================================================================

# Enable USB output, disable UART for stdio
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# =============================================================================
# OUTPUT FILES
# =============================================================================

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Set binary name
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "diagnostic_rig_pico"
)

# =============================================================================
# BUILD INFORMATION
# =============================================================================

# Print build configuration
message(STATUS "")
message(STATUS "=== Pico W Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Pico SDK Path: $ENV{PICO_SDK_PATH}")
message(STATUS "WiFi Support: ${ENABLE_WIFI}")

# Count and display source files
list(LENGTH PICO_PLATFORM_SOURCES PICO_SRC_COUNT)
list(LENGTH COMMON_SOURCES COMMON_SRC_COUNT)
math(EXPR TOTAL_SRC_COUNT "${PICO_SRC_COUNT} + ${COMMON_SRC_COUNT}")

message(STATUS "Platform Sources: ${PICO_SRC_COUNT}")
message(STATUS "Common Sources: ${COMMON_SRC_COUNT}")
message(STATUS "Total Sources: ${TOTAL_SRC_COUNT}")

# Print include directories for debugging
message(STATUS "Include Directories:")
foreach(DIR ${INCLUDE_DIRS})
    message(STATUS "  ${DIR}")
endforeach()

if(COMMON_SRC_COUNT EQUAL 0)
    message(WARNING "No common source files found! Most functionality will be missing.")
    message(STATUS "This is normal for a minimal build.")
endif()

message(STATUS "================================")
message(STATUS "")

# =============================================================================
# DEBUGGING HELPERS
# =============================================================================

# Option to print all included files
option(VERBOSE_BUILD "Print all source files being compiled" OFF)
if(VERBOSE_BUILD)
    message(STATUS "Platform source files:")
    foreach(SRC ${PICO_PLATFORM_SOURCES})
        message(STATUS "  ${SRC}")
    endforeach()
    
    message(STATUS "Common source files:")
    foreach(SRC ${COMMON_SOURCES})
        message(STATUS "  ${SRC}")
    endforeach()
endif()

# Custom target for cleaning build artifacts
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Removing entire build directory"
)

# Custom target for flashing
add_custom_target(flash
    COMMAND ${CMAKE_COMMAND} -E echo "Put Pico in BOOTSEL mode, then run:"
    COMMAND ${CMAKE_COMMAND} -E echo "cp ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2 /Volumes/RPI-RP2/"
    DEPENDS ${PROJECT_NAME}
    COMMENT "Instructions for flashing firmware"
)
