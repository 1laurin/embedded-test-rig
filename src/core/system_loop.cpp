/**
 * @file system_loop.cpp
 * @brief Main application loop implementation - COMPLETE VERSION
 */

#include "../system/system_loop.h"
#include "../ui/input_handler.h"
#include "../system/safety_monitor.h"
#include "../monitoring/diagnostics_engine.h"
#include "../utils/hal_interface.h"
#include "../include/board_config.h"
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static volatile bool system_stop_requested = false;
static uint32_t loop_counter = 0;
static uint32_t system_start_time = 0;

// =============================================================================
// PUBLIC FUNCTIONS - ALL FUNCTIONS IMPLEMENTED
// =============================================================================

void run_main_loop(void)
{
    printf("[LOOP] Starting main application loop...\n");

    system_start_time = hal_get_tick_ms();
    system_stop_requested = false;
    loop_counter = 0;

    while (!system_stop_requested)
    {
        loop_counter++;

        // Handle user input
        handle_user_input();

        // Perform safety checks
        check_system_safety();

        // Heartbeat task (blink LED)
        if (loop_counter % 1000 == 0)
        {
            hal_gpio_toggle(LED_STATUS_PIN);
            printf("[LOOP] Heartbeat: %lu loops\n", loop_counter);
        }

        // Test diagnostic channels periodically
        if (loop_counter % 5000 == 0)
        {
            test_diagnostic_channels();
        }

        hal_delay_ms(MAIN_LOOP_DELAY_MS);
    }

    printf("[LOOP] Main loop exiting after %lu iterations\n", loop_counter);
}

void request_system_stop(void)
{
    printf("[LOOP] System stop requested\n");
    system_stop_requested = true;
}

bool is_system_stop_requested(void)
{
    return system_stop_requested;
}

uint32_t get_loop_counter(void)
{
    return loop_counter;
}

void reset_loop_counter(void)
{
    loop_counter = 0;
}

void heartbeat_task(void)
{
    hal_gpio_toggle(LED_STATUS_PIN);
}

void update_system_status(void)
{
    printf("[STATUS] Loop count: %lu, Uptime: %lu ms\n",
           loop_counter, get_system_uptime_ms());
}

uint32_t get_system_uptime_ms(void)
{
    return hal_get_tick_ms() - system_start_time;
}

uint32_t get_system_uptime_seconds(void)
{
    return get_system_uptime_ms() / 1000;
}
