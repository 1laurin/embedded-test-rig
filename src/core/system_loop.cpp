/**
 * @file system_loop.cpp
 * @brief Main application loop implementation
 *
 * This file implements the main system loop and all periodic tasks
 * that run continuously during system operation.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "core/system_loop.h"
#include "ui/input_handler.h"
#include "system/safety_monitor.h"
#include "monitoring/diagnostics_engine.h"
#include "platforms/common/hal_interface.h"
#include "platforms/pico_w/include/board_config.h"
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static volatile bool system_stop_requested = false;
static uint32_t loop_counter = 0;
static uint32_t last_heartbeat_time = 0;
static uint32_t last_status_update = 0;
static uint32_t last_safety_check = 0;
static uint32_t last_diagnostics_test = 0;
static uint32_t system_start_time = 0;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Run the main application loop until system stops
 */
void run_main_loop(void)
{
    printf("[LOOP] Starting main application loop...\n");

    system_start_time = hal_get_tick_ms();
    system_stop_requested = false;
    loop_counter = 0;

    while (!system_stop_requested)
    {
        uint32_t loop_start_time = hal_get_tick_ms();

        // Increment loop counter
        loop_counter++;

        // Handle user input (buttons, commands)
        handle_user_input();

        // Perform safety checks
        uint32_t current_time = hal_get_tick_ms();
        if (current_time - last_safety_check >= SAFETY_CHECK_INTERVAL_MS)
        {
            check_system_safety();
            last_safety_check = current_time;
        }

        // Heartbeat task (blink LED, indicate system alive)
        heartbeat_task();

        // Update system status periodically
        if (current_time - last_status_update >= STATUS_UPDATE_INTERVAL_MS)
        {
            update_system_status();
            last_status_update = current_time;
        }

        // Run diagnostic tests periodically
        if (current_time - last_diagnostics_test >= (50 * MAIN_LOOP_DELAY_MS))
        { // Every 5 seconds
            test_diagnostic_channels();
            last_diagnostics_test = current_time;
        }

        // Sleep for the remainder of the loop time
        uint32_t loop_duration = hal_get_tick_ms() - loop_start_time;
        if (loop_duration < MAIN_LOOP_DELAY_MS)
        {
            hal_delay_ms(MAIN_LOOP_DELAY_MS - loop_duration);
        }
    }

    printf("[LOOP] Main loop exiting after %lu iterations\n", loop_counter);
}

/**
 * @brief Request the main loop to stop
 */
void request_system_stop(void)
{
    printf("[LOOP] System stop requested\n");
    system_stop_requested = true;
}

/**
 * @brief Check if system stop has been requested
 */
bool is_system_stop_requested(void)
{
    return system_stop_requested;
}

/**
 * @brief Get the current loop iteration counter
 */
uint32_t get_loop_counter(void)
{
    return loop_counter;
}

/**
 * @brief Reset the loop counter to zero
 */
void reset_loop_counter(void)
{
    loop_counter = 0;
}

/**
 * @brief Perform heartbeat task (typically blinks status LED)
 */
void heartbeat_task(void)
{
    uint32_t current_time = hal_get_tick_ms();

    if (current_time - last_heartbeat_time >= HEARTBEAT_INTERVAL_MS)
    {
        last_heartbeat_time = current_time;

        // Toggle status LED for heartbeat
        hal_gpio_toggle(LED_STATUS_PIN);

        // Print heartbeat message every 10 heartbeats (10 seconds)
        static uint8_t heartbeat_count = 0;
        heartbeat_count++;

        if (heartbeat_count >= 10)
        {
            printf("[HEARTBEAT] System alive - Uptime: %lu s, Loops: %lu\n",
                   get_system_uptime_seconds(), loop_counter);
            heartbeat_count = 0;
        }
    }
}

/**
 * @brief Update system status information
 */
void update_system_status(void)
{
    printf("[STATUS] Loop count: %lu, Uptime: %lu ms\n",
           loop_counter, get_system_uptime_ms());

    // Read and display ADC values
    uint16_t adc_value;
    float voltage;

    for (int i = 0; i < 3 && i < ADC_NUM_CHANNELS; i++)
    { // Read first 3 ADC channels
        if (hal_adc_read(i, &adc_value) == HAL_OK)
        {
            if (hal_adc_read_voltage(i, &voltage) == HAL_OK)
            {
                printf("[ADC] Channel %d: %u counts, %.3f V\n", i, adc_value, voltage);
            }
        }
    }

    // Update web display with status
    update_display_status();
}

/**
 * @brief Get system uptime in milliseconds
 */
uint32_t get_system_uptime_ms(void)
{
    return hal_get_tick_ms() - system_start_time;
}

/**
 * @brief Get system uptime in seconds
 */
uint32_t get_system_uptime_seconds(void)
{
    return get_system_uptime_ms() / 1000;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Update display with current system status
 */
static void update_display_status(void)
{
    // Clear display with dark blue background
    hal_display_clear(0x000080);

    // Title
    hal_display_draw_text(10, 10, "Pico W Diagnostic Rig", 0xFFFFFF, 0x000080);

    // System status
    char status_line[64];
    snprintf(status_line, sizeof(status_line), "Uptime: %lu s", get_system_uptime_seconds());
    hal_display_draw_text(10, 30, status_line, 0x00FF00, 0x000080);

    snprintf(status_line, sizeof(status_line), "Loop: %lu", loop_counter);
    hal_display_draw_text(10, 50, status_line, 0x00FF00, 0x000080);

    // Channel status - get from diagnostics engine
    hal_display_draw_text(10, 80, "Diagnostic Channels:", 0xFFFF00, 0x000080);

    bool channels_enabled[NUM_DIAGNOSTIC_CHANNELS];
    get_channel_states(channels_enabled); // This function would be in diagnostics_engine

    for (int i = 0; i < NUM_DIAGNOSTIC_CHANNELS; i++)
    {
        snprintf(status_line, sizeof(status_line), "Ch%d: %s",
                 i + 1, channels_enabled[i] ? "ENABLED" : "DISABLED");
        uint32_t color = channels_enabled[i] ? 0x00FF00 : 0xFF0000;
        hal_display_draw_text(20, 100 + i * 20, status_line, color, 0x000080);
    }

    // ADC readings
    hal_display_draw_text(10, 200, "ADC Readings:", 0xFFFF00, 0x000080);
    float voltage;
    if (hal_adc_read_voltage(0, &voltage) == HAL_OK)
    {
        snprintf(status_line, sizeof(status_line), "Ch1: %.3f V", voltage);
        hal_display_draw_text(20, 220, status_line, 0x00FFFF, 0x000080);
    }

    hal_display_flush();
}