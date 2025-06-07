/**
 * @file system_init.cpp
 * @brief System initialization implementation for multi-channel diagnostic test rig
 *
 * This file implements the system initialization sequence, including
 * HAL initialization and hardware subsystem setup.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "core/system_init.h"
#include "platforms/pico_w/include/board_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool system_initialized = false;

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static hal_status_t init_uart_subsystem(void);
static hal_status_t init_gpio_subsystem(void);
static hal_status_t init_adc_subsystem(void);
static hal_status_t init_display_subsystem(void);
static hal_status_t setup_interrupts(void);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize HAL and all hardware subsystems
 */
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
    status = init_gpio_subsystem();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: GPIO initialization failed\n");
        return status;
    }
    printf("[INIT] GPIO subsystem initialized successfully\n");

    // Step 3: Initialize UART subsystem
    printf("[INIT] Initializing UART subsystem...\n");
    status = init_uart_subsystem();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: UART initialization failed\n");
        return status;
    }
    printf("[INIT] UART subsystem initialized successfully\n");

    // Step 4: Initialize ADC subsystem
    printf("[INIT] Initializing ADC subsystem...\n");
    status = init_adc_subsystem();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: ADC initialization failed\n");
        return status;
    }
    printf("[INIT] ADC subsystem initialized successfully\n");

    // Step 5: Initialize display subsystem
    printf("[INIT] Initializing display subsystem...\n");
    status = init_display_subsystem();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: Display initialization failed\n");
        return status;
    }
    printf("[INIT] Display subsystem initialized successfully\n");

    // Step 6: Setup interrupts
    printf("[INIT] Setting up interrupts...\n");
    status = setup_interrupts();
    if (status != HAL_OK)
    {
        printf("[INIT] ERROR: Interrupt setup failed\n");
        return status;
    }
    printf("[INIT] Interrupts configured successfully\n");

    // Step 7: Final initialization tasks
    printf("[INIT] Performing final initialization tasks...\n");

    // Turn on power LED to indicate system is ready
    hal_gpio_write(LED_POWER_PIN, GPIO_HIGH);

    // Clear any pending interrupts
    // Platform-specific interrupt clearing would go here

    system_initialized = true;
    printf("[INIT] System initialization complete!\n");
    printf("===============================\n\n");

    return HAL_OK;
}

/**
 * @brief Deinitialize all systems and cleanup resources
 */
hal_status_t system_deinit(void)
{
    if (!system_initialized)
    {
        return HAL_OK; // Already deinitialized
    }

    printf("\n[DEINIT] Starting system shutdown...\n");

    // Turn off all LEDs except power LED
    hal_gpio_write(LED_STATUS_PIN, GPIO_LOW);
    hal_gpio_write(LED_ERROR_PIN, GPIO_LOW);
    hal_gpio_write(LED_COMM_PIN, GPIO_LOW);

    // Clear display
    hal_display_clear(0x0000);
    hal_display_flush();

    // Disable all diagnostic channels
    for (int i = 1; i <= NUM_DIAGNOSTIC_CHANNELS; i++)
    {
        // Platform-specific channel disable would be called here
        // pico_set_channel_enable(i, false);
    }

    // Deinitialize HAL subsystems
    hal_display_deinit();
    hal_uart_deinit(0);
    hal_uart_deinit(1);
    hal_deinit();

    system_initialized = false;
    printf("[DEINIT] System shutdown complete\n");

    return HAL_OK;
}

/**
 * @brief Get system initialization status
 */
bool is_system_initialized(void)
{
    return system_initialized;
}

/**
 * @brief Print initialization progress and results
 */
void print_init_progress(void)
{
    printf("\n=== Initialization Status ===\n");
    printf("System Initialized: %s\n", system_initialized ? "YES" : "NO");

    if (system_initialized)
    {
        printf("Available Subsystems:\n");
        printf("  - GPIO: Ready\n");
        printf("  - UART: Ready (Debug + External)\n");
        printf("  - ADC: Ready (%d channels)\n", ADC_NUM_CHANNELS);
        printf("  - Display: Ready (%dx%d)\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);
        printf("  - Diagnostic Channels: %d available\n", NUM_DIAGNOSTIC_CHANNELS);

#if BOARD_HAS_WIFI
        printf("  - WiFi: Available\n");
#endif

#if BOARD_HAS_CAN
        printf("  - CAN: Available\n");
#endif

#if BOARD_HAS_DAC
        printf("  - DAC: Available\n");
#endif
    }
    printf("=============================\n\n");
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Initialize UART subsystem
 */
static hal_status_t init_uart_subsystem(void)
{
    hal_status_t status;

    // Initialize debug UART
    uart_config_t debug_uart_config = {
        .baudrate = UART_DEBUG_BAUDRATE,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0,
        .flow_control = false};

    status = hal_uart_init(0, &debug_uart_config);
    if (status != HAL_OK)
    {
        printf("Debug UART initialization failed\n");
        return status;
    }

    // Initialize external UART
    uart_config_t ext_uart_config = {
        .baudrate = UART_EXT_BAUDRATE,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0,
        .flow_control = false};

    status = hal_uart_init(1, &ext_uart_config);
    if (status != HAL_OK)
    {
        printf("External UART initialization failed\n");
        return status;
    }

    return HAL_OK;
}

/**
 * @brief Initialize GPIO subsystem
 */
static hal_status_t init_gpio_subsystem(void)
{
    hal_status_t status;

    status = hal_gpio_init();
    if (status != HAL_OK)
    {
        return status;
    }

    // Configure additional GPIO pins as needed
    // All basic pin configuration is done in hal_gpio_init()

    return HAL_OK;
}

/**
 * @brief Initialize ADC subsystem
 */
static hal_status_t init_adc_subsystem(void)
{
    hal_status_t status;

    status = hal_adc_init();
    if (status != HAL_OK)
    {
        return status;
    }

    // Configure ADC channels
    adc_config_t adc_config = {
        .channel = 0,
        .resolution_bits = ADC_RESOLUTION_BITS,
        .reference_voltage = ADC_REFERENCE_VOLTAGE,
        .sample_time_us = ADC_SAMPLE_TIME_US};

    // Configure each ADC channel
    for (int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
        adc_config.channel = i;
        status = hal_adc_config(&adc_config);
        if (status != HAL_OK)
        {
            printf("ADC channel %d configuration failed\n", i);
            return status;
        }
    }

    return HAL_OK;
}

/**
 * @brief Initialize display subsystem
 */
static hal_status_t init_display_subsystem(void)
{
    hal_status_t status;

    status = hal_display_init();
    if (status != HAL_OK)
    {
        return status;
    }

    // Set initial display state
    hal_display_clear(0x000000);     // Black background
    hal_display_set_brightness(100); // Full brightness

    return HAL_OK;
}

/**
 * @brief Setup interrupt handlers
 */
static hal_status_t setup_interrupts(void)
{
    hal_status_t status;

    // Setup user button interrupt - this would normally call a callback
    // in the input handler module
    extern void user_button_callback(uint32_t pin); // Forward declaration

    status = hal_gpio_interrupt_enable(BTN_USER_PIN, 2, user_button_callback); // Falling edge
    if (status != HAL_OK)
    {
        printf("User button interrupt setup failed\n");
        return status;
    }

    // Setup additional interrupts as needed
    // Emergency stop button, safety sensors, etc.

    return HAL_OK;
}