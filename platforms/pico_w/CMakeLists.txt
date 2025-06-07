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
