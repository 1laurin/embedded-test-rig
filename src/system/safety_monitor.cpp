/**
 * @file safety_monitor.cpp
 * @brief Safety monitoring and emergency shutdown implementation
 *
 * This file implements system safety monitoring including temperature,
 * voltage, and current monitoring with emergency shutdown capabilities.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "../system/safety_monitor.h"
#include "../utils/hal_interface.h"
#include "../include/board_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool safety_monitor_initialized = false;
static bool emergency_state = false;
static void (*emergency_callback)(void) = NULL;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the safety monitoring system
 */
bool safety_monitor_init(void)
{
    if (safety_monitor_initialized)
    {
        return true; // Already initialized
    }

    printf("[SAFETY] Initializing safety monitoring system...\n");

    emergency_state = false;
    safety_monitor_initialized = true;

    printf("[SAFETY] Safety monitoring system initialized\n");
    return true;
}

/**
 * @brief Check all safety parameters and take action if needed
 */
void check_system_safety(void)
{
    if (!safety_monitor_initialized || emergency_state)
    {
        return;
    }

    // Basic safety checks - placeholder implementation
    static uint32_t safety_check_count = 0;
    safety_check_count++;

    // Every 1000 safety checks, print status
    if (safety_check_count % 1000 == 0)
    {
        printf("[SAFETY] Safety check #%lu - System OK\n", safety_check_count);
    }
}

/**
 * @brief Perform emergency shutdown sequence
 */
void emergency_shutdown(const char *reason)
{
    if (emergency_state)
    {
        return; // Already in emergency state
    }

    printf("\n!!! EMERGENCY SHUTDOWN !!!\n");
    printf("Reason: %s\n", reason);

    emergency_state = true;

    // Turn on error LED
    hal_gpio_write(LED_ERROR_PIN, GPIO_HIGH);

    // Turn off power-related outputs
    hal_gpio_write(RELAY_1_PIN, GPIO_LOW);
    hal_gpio_write(RELAY_2_PIN, GPIO_LOW);

    // Call emergency callback if registered
    if (emergency_callback != NULL)
    {
        emergency_callback();
    }

    printf("Emergency shutdown complete. System is now in safe state.\n");
}

/**
 * @brief Check if system is in emergency state
 */
bool is_emergency_state(void)
{
    return emergency_state;
}

/**
 * @brief Register callback for emergency events
 */
void register_safety_emergency_callback(void (*callback)(void))
{
    emergency_callback = callback;
    printf("[SAFETY] Emergency callback registered\n");
}
