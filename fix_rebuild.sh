#!/bin/bash

# Fix the main.cpp linker issues and ensure clean build
set -e

echo "🔧 Fixing main.cpp and ensuring clean rebuild..."

# 1. Fix unused parameter warnings first
echo "📝 Fixing unused parameter warnings..."
sed -i '' 's/input_event_t \*event)/input_event_t *event __attribute__((unused)))/' src/ui/input_handler.cpp
sed -i '' 's/button_id_t button_id)/button_id_t button_id __attribute__((unused)))/' src/ui/input_handler.cpp

# 2. Create a completely new, working main.cpp that properly links with our functions
echo "📝 Creating new main.cpp with correct function calls..."
cat > platforms/pico_w/src/main.cpp << 'EOF'
/**
 * @file main.cpp
 * @brief Main application entry point for Raspberry Pi Pico W
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "system_init.h"
#include "system_loop.h"
#include "system_info.h"
#include "input_handler.h"
#include "safety_monitor.h"
#include "hal_demo.h"
#include "hal_test.h"
#include "board_config.h"
#include <stdio.h>

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void print_startup_banner(void);
static void setup_emergency_stop(void);
static void system_emergency_stop_handler(void);
static void cleanup_and_exit(void);

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main()
{
    // Print startup banner
    print_startup_banner();

    // Initialize the entire system
    printf("Initializing system...\n");
    hal_status_t init_status = system_init();
    if (init_status != HAL_OK)
    {
        printf("CRITICAL ERROR: System initialization failed (status: %d)\n", init_status);
        printf("System cannot continue. Please check hardware connections.\n");
        return -1;
    }

    // Initialize input handler
    if (!input_handler_init())
    {
        printf("ERROR: Input handler initialization failed\n");
        cleanup_and_exit();
        return -2;
    }

    // Setup emergency stop handling
    setup_emergency_stop();

    // Print system information
    display_system_info();
    print_init_progress();

    // Run HAL feature demonstration
    printf("Running HAL demonstration...\n");
    run_hal_demo();

    // Test all HAL subsystems
    printf("Testing HAL subsystems...\n");
    if (!test_hal_subsystems())
    {
        printf("WARNING: Some HAL subsystem tests failed\n");
        printf("System will continue, but functionality may be limited\n");
    }

    printf("\n");
    printf("=================================================\n");
    printf(" System Ready - Starting Main Application Loop\n");
    printf("=================================================\n");
    printf("Press the user button to toggle diagnostic channels\n");
    printf("Send UART commands for remote control\n");
    printf("System will perform automatic safety monitoring\n");
    printf("\n");

    // Enter the main application loop
    run_main_loop();

    // If we reach here, the system is shutting down
    cleanup_and_exit();
    return 0;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Print the startup banner
 */
static void print_startup_banner(void)
{
    printf("\n");
    printf("=======================================================\n");
    printf("  Multi-Channel Diagnostic Test Rig - Pico W Edition\n");
    printf("=======================================================\n");
    printf("Version: 1.0.0\n");
    printf("Platform: Raspberry Pi Pico W\n");
    printf("MCU: RP2040 @ 125 MHz\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("=======================================================\n");
    printf("\n");
}

/**
 * @brief Setup emergency stop handling
 */
static void setup_emergency_stop(void)
{
    printf("[MAIN] Setting up emergency stop handling...\n");

    // Register emergency stop callback with input handler
    register_emergency_stop_callback(system_emergency_stop_handler);

    // Register emergency stop callback with safety monitor
    register_safety_emergency_callback(system_emergency_stop_handler);

    printf("[MAIN] Emergency stop handling configured\n");
}

/**
 * @brief Emergency stop handler - called when emergency stop is triggered
 */
static void system_emergency_stop_handler(void)
{
    printf("\n");
    printf("!!! EMERGENCY STOP TRIGGERED !!!\n");
    printf("Initiating immediate system shutdown...\n");

    // Perform emergency shutdown
    emergency_shutdown("Emergency stop button pressed");

    // Request main loop to stop
    request_system_stop();
}

/**
 * @brief Cleanup and exit the application
 */
static void cleanup_and_exit(void)
{
    printf("\n[MAIN] Starting system cleanup...\n");

    // Disable input processing
    set_input_processing_enabled(false);

    // Clear any pending input events
    clear_input_events();

    // Deinitialize all systems
    hal_status_t deinit_status = system_deinit();
    if (deinit_status != HAL_OK)
    {
        printf("[MAIN] WARNING: System deinitialization had errors (status: %d)\n", deinit_status);
    }

    printf("[MAIN] System cleanup complete\n");
    printf("[MAIN] Application exiting\n");
    printf("\n");
    printf("=======================================================\n");
    printf("  Multi-Channel Diagnostic Test Rig - Shutdown Complete\n");
    printf("=======================================================\n");
}
EOF

# 3. Force a complete clean rebuild
echo "🧹 Performing clean rebuild..."
cd build/pico_w

# Remove all build artifacts
rm -rf *

# Reconfigure CMake
echo "🔧 Reconfiguring CMake..."
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DPICO_BOARD=pico_w \
    -DPICO_PLATFORM=rp2040 \
    ../../platforms/pico_w

# Build with verbose output to see exactly what's happening
echo "🔨 Building with verbose output..."
make VERBOSE=1 -j4

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 BUILD SUCCESSFUL! 🎉"
    echo ""
    echo "📦 Generated files:"
    ls -la *.uf2 *.elf 2>/dev/null || echo "No output files found"
    echo ""
    echo "🚀 To flash to Pico W:"
    echo "1. Hold BOOTSEL button while connecting Pico W to USB"
    echo "2. Copy diagnostic_rig_pico.uf2 to the RPI-RP2 drive"
    echo "3. Pico W will reboot and run your firmware!"
    echo ""
    echo "📟 To monitor output:"
    echo "   ./monitor.sh pico"
else
    echo ""
    echo "❌ Build failed. Let's check what's still missing..."
    echo ""
    echo "🔍 Checking for remaining undefined references..."
    make 2>&1 | grep "undefined reference" | head -5
fi