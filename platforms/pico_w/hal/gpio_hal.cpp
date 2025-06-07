/**
 * @file gpio_hal.cpp
 * @brief GPIO Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

static bool gpio_initialized = false;

hal_status_t hal_gpio_init(void) {
    if (gpio_initialized) {
        return HAL_OK;
    }
    
    printf("[GPIO] Initializing GPIO subsystem...\n");
    
    // Configure status LEDs as outputs
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
    
    // Configure buttons as inputs with pull-ups
    gpio_init(BTN_USER_PIN);
    gpio_set_dir(BTN_USER_PIN, GPIO_IN);
    gpio_pull_up(BTN_USER_PIN);
    
    gpio_init(BTN_RESET_PIN);
    gpio_set_dir(BTN_RESET_PIN, GPIO_IN);
    gpio_pull_up(BTN_RESET_PIN);
    
    gpio_init(BTN_MODE_PIN);
    gpio_set_dir(BTN_MODE_PIN, GPIO_IN);
    gpio_pull_up(BTN_MODE_PIN);
    
    gpio_initialized = true;
    printf("[GPIO] GPIO subsystem initialized\n");
    
    return HAL_OK;
}

hal_status_t hal_gpio_config(uint32_t pin, gpio_mode_t mode) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_init(pin);
    
    switch (mode) {
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
            // Pico doesn't have true open drain, simulate it
            gpio_set_dir(pin, GPIO_OUT);
            break;
        default:
            return HAL_INVALID_PARAM;
    }
    
    return HAL_OK;
}

hal_status_t hal_gpio_write(uint32_t pin, gpio_state_t state) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_put(pin, (state == GPIO_HIGH) ? 1 : 0);
    return HAL_OK;
}

hal_status_t hal_gpio_read(uint32_t pin, gpio_state_t *state) {
    if (!gpio_initialized || !state) {
        return HAL_ERROR;
    }
    
    *state = gpio_get(pin) ? GPIO_HIGH : GPIO_LOW;
    return HAL_OK;
}

hal_status_t hal_gpio_toggle(uint32_t pin) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_put(pin, !gpio_get(pin));
    return HAL_OK;
}

hal_status_t hal_gpio_interrupt_enable(uint32_t pin, uint8_t trigger_edge, void (*callback)(uint32_t)) {
    if (!gpio_initialized || !callback) {
        return HAL_ERROR;
    }
    
    // Store callback (simplified - real implementation would need callback storage)
    uint32_t events = 0;
    if (trigger_edge & 1) events |= GPIO_IRQ_EDGE_RISE;
    if (trigger_edge & 2) events |= GPIO_IRQ_EDGE_FALL;
    
    gpio_set_irq_enabled_with_callback(pin, events, true, (gpio_irq_callback_t)callback);
    
    return HAL_OK;
}

hal_status_t hal_gpio_interrupt_disable(uint32_t pin) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    return HAL_OK;
}
