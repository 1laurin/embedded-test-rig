/**
 * @file hal_main.cpp
 * @brief Main HAL initialization and system functions for Raspberry Pi Pico W
 *
 * This file implements the main HAL functions including system initialization,
 * timing, and global HAL management.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "../utils/hal_interface.h"
#include "../include/board_config.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool hal_system_initialized = false;
static uint32_t system_start_time = 0;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the HAL layer
 * @return HAL status code
 */
hal_status_t hal_init(void)
{
    if (hal_system_initialized)
    {
        return HAL_OK; // Already initialized
    }

    printf("[HAL] Initializing Pico W HAL layer...\n");

    // Initialize standard I/O and basic clocks
    stdio_init_all();

    // Store system start time
    system_start_time = to_ms_since_boot(get_absolute_time());

#ifdef WATCHDOG_TIMEOUT_MS
    if (WATCHDOG_TIMEOUT_MS > 0)
    {
        watchdog_enable(WATCHDOG_TIMEOUT_MS, 1);
        printf("[HAL] Watchdog enabled with %d ms timeout\n", WATCHDOG_TIMEOUT_MS);
    }
#endif

    hal_system_initialized = true;

    printf("[HAL] Pico W HAL layer initialized successfully\n");
    printf("[HAL] System clock: %lu Hz\n", clock_get_hz(clk_sys));
    printf("[HAL] Peripheral clock: %lu Hz\n", clock_get_hz(clk_peri));

    return HAL_OK;
}

/**
 * @brief Deinitialize the HAL layer
 * @return HAL status code
 */
hal_status_t hal_deinit(void)
{
    if (!hal_system_initialized)
    {
        return HAL_OK; // Already deinitialized
    }

    printf("[HAL] Deinitializing HAL layer...\n");

#ifdef WATCHDOG_TIMEOUT_MS
    watchdog_enable(0, 0);
#endif

    hal_system_initialized = false;

    printf("[HAL] HAL layer deinitialized\n");

    return HAL_OK;
}

/**
 * @brief Get system tick count in milliseconds
 * @return Current tick count
 */
uint32_t hal_get_tick_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

/**
 * @brief Delay execution for specified milliseconds
 * @param ms Delay time in milliseconds
 */
void hal_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

/**
 * @brief Delay execution for specified microseconds
 * @param us Delay time in microseconds
 */
void hal_delay_us(uint32_t us)
{
    sleep_us(us);
}

/**
 * @brief Reset the system
 */
void hal_system_reset(void)
{
    printf("[HAL] System reset requested\n");
    // Add system reset code here if needed
}
