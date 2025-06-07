/**
 * @file system_loop.h
 * @brief Main application loop and periodic tasks interface
 *
 * This file provides the interface for the main application loop
 * and all periodic tasks that need to run continuously.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef SYSTEM_LOOP_H
#define SYSTEM_LOOP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // CONFIGURATION CONSTANTS
    // =============================================================================

#define MAIN_LOOP_DELAY_MS 100
#define HEARTBEAT_INTERVAL_MS 1000
#define STATUS_UPDATE_INTERVAL_MS 5000
#define SAFETY_CHECK_INTERVAL_MS 500

    // =============================================================================
    // PUBLIC FUNCTIONS
    // =============================================================================

    /**
     * @brief Run the main application loop until system stops
     */
    void run_main_loop(void);

    /**
     * @brief Request the main loop to stop
     */
    void request_system_stop(void);

    /**
     * @brief Check if system stop has been requested
     * @return true if stop requested, false otherwise
     */
    bool is_system_stop_requested(void);

    /**
     * @brief Get the current loop iteration counter
     * @return Number of main loop iterations completed
     */
    uint32_t get_loop_counter(void);

    /**
     * @brief Reset the loop counter to zero
     */
    void reset_loop_counter(void);

    /**
     * @brief Perform heartbeat task (typically blinks status LED)
     */
    void heartbeat_task(void);

    /**
     * @brief Update system status information
     */
    void update_system_status(void);

    /**
     * @brief Get system uptime in milliseconds
     * @return System uptime since initialization
     */
    uint32_t get_system_uptime_ms(void);

    /**
     * @brief Get system uptime in seconds
     * @return System uptime since initialization
     */
    uint32_t get_system_uptime_seconds(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_LOOP_H
