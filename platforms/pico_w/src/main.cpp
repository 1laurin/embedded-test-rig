/**
 * @file main.cpp
 * @brief Main application entry point for Raspberry Pi Pico W (Refactored)
 * 
 * This file contains the clean, refactored main application that uses
 * the modular system components for better organization and maintainability.
 * 
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

 #include "../../src/core/system_init.h"
 #include "../../src/core/system_loop.h"
 #include "../../src/core/system_info.h"
 #include "../../src/ui/input_handler.h"
 #include "../../src/system/safety_monitor.h"
 #include "../../src/demo/hal_demo.h"
 #include "../../src/utils/hal_test.h"
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
 
 int main() {
     // Print startup banner
     print_startup_banner();
 
     // Initialize the entire system
     printf("Initializing system...\n");
     hal_status_t init_status = system_init();
     if (init_status != HAL_OK) {
         printf("CRITICAL ERROR: System initialization failed (status: %d)\n", init_status);
         printf("System cannot continue. Please check hardware connections.\n");
         return -1;
     }
 
     // Initialize input handler
     if (!input_handler_init()) {
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
     if (!test_hal_subsystems()) {
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
     // This function will run indefinitely until system stop is requested
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
 static void print_startup_banner(void) {
     printf("\n");
     printf("=======================================================\n");
     printf("  Multi-Channel Diagnostic Test Rig - Pico W Edition\n");
     printf("=======================================================\n");
     printf("Version: 1.0.0\n");
     printf("Platform: %s\n", BOARD_NAME);
     printf("MCU: %s @ %lu MHz\n", MCU_NAME, MCU_FREQUENCY_HZ / 1000000);
     printf("Build Date: %s %s\n", __DATE__, __TIME__);
     printf("=======================================================\n");
     printf("\n");
 }
 
 /**
  * @brief Setup emergency stop handling
  */
 static void setup_emergency_stop(void) {
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
 static void system_emergency_stop_handler(void) {
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
 static void cleanup_and_exit(void) {
     printf("\n[MAIN] Starting system cleanup...\n");
     
     // Disable input processing
     set_input_processing_enabled(false);
     
     // Clear any pending input events
     clear_input_events();
     
     // Deinitialize all systems
     hal_status_t deinit_status = system_deinit();
     if (deinit_status != HAL_OK) {
         printf("[MAIN] WARNING: System deinitialization had errors (status: %d)\n", deinit_status);
     }
     
     printf("[MAIN] System cleanup complete\n");
     printf("[MAIN] Application exiting\n");
     printf("\n");
     printf("=======================================================\n");
     printf("  Multi-Channel Diagnostic Test Rig - Shutdown Complete\n");
     printf("=======================================================\n");
 }