/**
 * @file gpio_hal.cpp
 * @brief GPIO Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 *
 * This file implements the GPIO HAL interface for the Pico W platform using
 * the Pico SDK GPIO functions.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool gpio_subsystem_initialized = false;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize GPIO subsystem
 * @return HAL status code
 */
hal_status_t hal_gpio_init(void)
{
    if (gpio_subsystem_initialized)
    {
        return HAL_OK; // Already initialized
    }

    printf("[GPIO] Initializing GPIO subsystem...\n");

    // Initialize all required GPIO pins
    
    // Status LEDs
    gpio_init(LED_STATUS_PIN);
    gpio_set_dir(LED_STATUS_PIN, GPIO_OUT);
    gpio_put(LED_STATUS_PIN, 0);

    gpio_init(LED_ERROR_PIN);
    gpio_set_dir(LED_ERROR_PIN, GPIO_OUT);
    gpio_put(LED_ERROR_PIN, 0);

    gpio_init(LED_COMM_PIN);
    gpio_set_dir(LED_COMM_PIN, GPIO_OUT);
    gpio_put(LED_COMM_PIN, 0);

    gpio_init(LED_POWER_PIN);
    gpio_set_dir(LED_POWER_PIN, GPIO_OUT);
    gpio_put(LED_POWER_PIN, 1); // Power LED on

    // User buttons (with pull-ups)
    gpio_init(BTN_USER_PIN);
    gpio_set_dir(BTN_USER_PIN, GPIO_IN);
    gpio_pull_up(BTN_USER_PIN);

    // Control outputs
    gpio_init(RELAY_1_PIN);
    gpio_set_dir(RELAY_1_PIN, GPIO_OUT);
    gpio_put(RELAY_1_PIN, 0);

    gpio_init(RELAY_2_PIN);
    gpio_set_dir(RELAY_2_PIN, GPIO_OUT);
    gpio_put(RELAY_2_PIN, 0);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    gpio_subsystem_initialized = true;

    printf("[GPIO] GPIO subsystem initialized successfully\n");

    return HAL_OK;
}

/**
 * @brief Configure a GPIO pin
 * @param pin Pin number (platform-specific)
 * @param mode GPIO mode
 * @return HAL status code
 */
hal_status_t hal_gpio_config(uint32_t pin, gpio_mode_t mode)
{
    if (!gpio_subsystem_initialized)
    {
        return HAL_ERROR;
    }

    gpio_init(pin);

    switch (mode)
    {
    case GPIO_INPUT:
        gpio_set_dir(pin, GPIO_IN);
        gpio_disable_pulls(pin);
        break;
    case GPIO_OUTPUT:
        gpio_set_dir(pin, GPIO_OUT);
        break;
    case GPIO_INPUT_PULLUP:
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
        break;
    case GPIO_INPUT_PULLDOWN:
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
        break;
    case GPIO_OPEN_DRAIN:
        // Pico doesn't have true open drain, simulate with direction changes
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 0);
        break;
    default:
        return HAL_INVALID_PARAM;
    }

    return HAL_OK;
}

/**
 * @brief Write to a GPIO pin
 * @param pin Pin number
 * @param state Pin state (HIGH/LOW)
 * @return HAL status code
 */
hal_status_t hal_gpio_write(uint32_t pin, gpio_state_t state)
{
    if (!gpio_subsystem_initialized)
    {
        return HAL_ERROR;
    }

    gpio_put(pin, (state == GPIO_HIGH) ? 1 : 0);
    return HAL_OK;
}

/**
 * @brief Read from a GPIO pin
 * @param pin Pin number
 * @param state Pointer to store pin state
 * @return HAL status code
 */
hal_status_t hal_gpio_read(uint32_t pin, gpio_state_t *state)
{
    if (!gpio_subsystem_initialized || state == nullptr)
    {
        return HAL_ERROR;
    }

    *state = gpio_get(pin) ? GPIO_HIGH : GPIO_LOW;
    return HAL_OK;
}

/**
 * @brief Toggle a GPIO pin
 * @param pin Pin number
 * @return HAL status code
 */
hal_status_t hal_gpio_toggle(uint32_t pin)
{
    if (!gpio_subsystem_initialized)
    {
        return HAL_ERROR;
    }

    gpio_put(pin, !gpio_get(pin));
    return HAL_OK;
}

/**
 * @brief Enable GPIO interrupt
 * @param pin Pin number
 * @param trigger_edge Edge type (rising=1, falling=2, both=3)
 * @param callback Interrupt callback function
 * @return HAL status code
 */
hal_status_t hal_gpio_interrupt_enable(uint32_t pin, uint8_t trigger_edge, void (*callback)(uint32_t pin))
{
    if (!gpio_subsystem_initialized || callback == nullptr)
    {
        return HAL_ERROR;
    }

    uint32_t events = 0;
    if (trigger_edge & 1) events |= GPIO_IRQ_EDGE_RISE;
    if (trigger_edge & 2) events |= GPIO_IRQ_EDGE_FALL;

    gpio_set_irq_enabled_with_callback(pin, events, true, (gpio_irq_callback_t)callback);

    return HAL_OK;
}

/**
 * @brief Disable GPIO interrupt
 * @param pin Pin number
 * @return HAL status code
 */
hal_status_t hal_gpio_interrupt_disable(uint32_t pin)
{
    if (!gpio_subsystem_initialized)
    {
        return HAL_ERROR;
    }

    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    return HAL_OK;
}
