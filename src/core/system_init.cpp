/**
 * @file system_init.cpp
 * @brief System initialization implementation for multi-channel diagnostic test rig
 */

#include "../system/system_init.h"
#include "../monitoring/diagnostics_engine.h"
#include "../include/board_config.h"
#include <stdio.h>
#include <string.h>

static bool system_initialized = false;

hal_status_t system_init(void)
{
    hal_status_t status;

    if (system_initialized)
    {
        printf("[INIT] System already initialized\n");
        return HAL_OK;
    }

    printf("\n=== System Initialization ===\n");

    // Step 1: Initialize HAL layer
    printf("[INIT] Initializing HAL layer...\n");
    status = hal_init();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: HAL initialization failed (status: %d)\n", status);
        return status;
    }
    printf("[INIT] HAL layer initialized successfully\n");

    // Step 2: Initialize GPIO subsystem
    printf("[INIT] Initializing GPIO subsystem...\n");
    status = hal_gpio_init();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: GPIO initialization failed\n");
        return status;
    }
    printf("[INIT] GPIO subsystem initialized successfully\n");

    // Step 3: Initialize ADC subsystem
    printf("[INIT] Initializing ADC subsystem...\n");
    status = hal_adc_init();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: ADC initialization failed\n");
        return status;
    }
    printf("[INIT] ADC subsystem initialized successfully\n");

    // Step 4: Initialize display subsystem
    printf("[INIT] Initializing display subsystem...\n");
    status = hal_display_init();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: Display initialization failed\n");
        return status;
    }
    printf("[INIT] Display subsystem initialized successfully\n");

    // Step 5: Initialize diagnostics engine
    printf("[INIT] Initializing diagnostics engine...\n");
    if (!diagnostics_engine_init())
    {
        printf("[INIT] ERROR: Diagnostics engine initialization failed\n");
        return HAL_ERROR;
    }
    printf("[INIT] Diagnostics engine initialized successfully\n");

    // Turn on power LED to indicate system is ready
    hal_gpio_write(LED_POWER_PIN, GPIO_HIGH);

    system_initialized = true;
    printf("[INIT] System initialization complete!\n");
    printf("===============================\n\n");

    return HAL_OK;
}

hal_status_t system_deinit(void)
{
    if (!system_initialized)
    {
        return HAL_OK;
    }

    printf("\n[DEINIT] Starting system shutdown...\n");

    // Deinitialize diagnostics engine
    diagnostics_engine_deinit();

    // Turn off all LEDs except power LED
    hal_gpio_write(LED_STATUS_PIN, GPIO_LOW);
    hal_gpio_write(LED_ERROR_PIN, GPIO_LOW);
    hal_gpio_write(LED_COMM_PIN, GPIO_LOW);

    // Clear display
    hal_display_clear(0x0000);
    hal_display_flush();

    // Deinitialize HAL subsystems
    hal_display_deinit();
    hal_deinit();

    system_initialized = false;
    printf("[DEINIT] System shutdown complete\n");

    return HAL_OK;
}

bool is_system_initialized(void)
{
    return system_initialized;
}

void print_init_progress(void)
{
    printf("\n=== Initialization Status ===\n");
    printf("System Initialized: %s\n", system_initialized ? "YES" : "NO");

    if (system_initialized)
    {
        printf("Available Subsystems:\n");
        printf("  - GPIO: Ready\n");
        printf("  - ADC: Ready (%d channels)\n", ADC_NUM_CHANNELS);
        printf("  - Display: Ready (%dx%d)\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
        printf("  - Diagnostic Engine: Ready\n");
        printf("  - Diagnostic Channels: %d available\n", NUM_DIAGNOSTIC_CHANNELS);
    }
    printf("=============================\n\n");
}
