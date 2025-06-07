#!/bin/bash

# Create missing HAL implementations for Pico W
# This script creates the missing HAL files that are referenced but empty

set -e

echo "Creating missing Pico W HAL implementations..."

# Create proper GPIO HAL implementation
cat > platforms/pico_w/hal/gpio_hal.cpp << 'EOF'
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

#include "../../platforms/common/hal_interface.h"
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
EOF

# Create proper ADC HAL implementation
cat > platforms/pico_w/hal/adc_hal.cpp << 'EOF'
/**
 * @file adc_hal.cpp
 * @brief ADC Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 *
 * This file implements the ADC HAL interface for the Pico W platform using
 * the Pico SDK ADC functions.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool adc_subsystem_initialized = false;

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize ADC subsystem
 * @return HAL status code
 */
hal_status_t hal_adc_init(void)
{
    if (adc_subsystem_initialized)
    {
        return HAL_OK; // Already initialized
    }

    printf("[ADC] Initializing ADC subsystem...\n");

    // Initialize ADC
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(ADC_CH1_VOLTAGE_PIN);
    adc_gpio_init(ADC_CH2_VOLTAGE_PIN);
    adc_gpio_init(ADC_CH3_CURRENT_PIN);
    // Note: Temperature sensor doesn't need GPIO init

    adc_subsystem_initialized = true;

    printf("[ADC] ADC subsystem initialized successfully\n");

    return HAL_OK;
}

/**
 * @brief Configure ADC channel
 * @param config ADC configuration structure
 * @return HAL status code
 */
hal_status_t hal_adc_config(const adc_config_t *config)
{
    if (!adc_subsystem_initialized || config == nullptr)
    {
        return HAL_ERROR;
    }

    // Pico ADC is fairly fixed in configuration
    // Just validate that the channel is reasonable
    if (config->channel > 4)
    {
        return HAL_INVALID_PARAM;
    }

    return HAL_OK;
}

/**
 * @brief Read ADC value (blocking)
 * @param channel ADC channel number
 * @param value Pointer to store ADC reading
 * @return HAL status code
 */
hal_status_t hal_adc_read(uint8_t channel, uint16_t *value)
{
    if (!adc_subsystem_initialized || value == nullptr)
    {
        return HAL_ERROR;
    }

    if (channel > 4)
    {
        return HAL_INVALID_PARAM;
    }

    // Select ADC input channel
    adc_select_input(channel);

    // Read ADC (12-bit result)
    *value = adc_read();

    return HAL_OK;
}

/**
 * @brief Read ADC value as voltage
 * @param channel ADC channel number
 * @param voltage Pointer to store voltage reading
 * @return HAL status code
 */
hal_status_t hal_adc_read_voltage(uint8_t channel, float *voltage)
{
    if (!adc_subsystem_initialized || voltage == nullptr)
    {
        return HAL_ERROR;
    }

    uint16_t adc_value;
    hal_status_t status = hal_adc_read(channel, &adc_value);

    if (status == HAL_OK)
    {
        // Convert to voltage (3.3V reference, 12-bit ADC)
        *voltage = (float)adc_value * ADC_REFERENCE_VOLTAGE / 4095.0f;
    }

    return status;
}

/**
 * @brief Start continuous ADC conversion
 * @param channel ADC channel number
 * @param callback Conversion complete callback
 * @return HAL status code
 */
hal_status_t hal_adc_start_continuous(uint8_t channel, void (*callback)(uint8_t channel, uint16_t value))
{
    // Not implemented for this simple version
    return HAL_NOT_SUPPORTED;
}

/**
 * @brief Stop continuous ADC conversion
 * @param channel ADC channel number
 * @return HAL status code
 */
hal_status_t hal_adc_stop_continuous(uint8_t channel)
{
    // Not implemented for this simple version
    return HAL_NOT_SUPPORTED;
}
EOF

# Create minimal stubs for other HAL files
echo "Creating minimal HAL stubs..."

cat > platforms/pico_w/hal/i2c_hal.cpp << 'EOF'
/**
 * @file i2c_hal.cpp
 * @brief I2C HAL stub for Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_i2c_init(uint8_t i2c_id, const i2c_config_t *config) {
    printf("[I2C] I2C%d init stub\n", i2c_id);
    return HAL_OK;
}

hal_status_t hal_i2c_deinit(uint8_t i2c_id) {
    return HAL_OK;
}

hal_status_t hal_i2c_transmit(uint8_t i2c_id, uint8_t device_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_receive(uint8_t i2c_id, uint8_t device_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_write_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_read_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}
EOF

cat > platforms/pico_w/hal/spi_hal.cpp << 'EOF'
/**
 * @file spi_hal.cpp
 * @brief SPI HAL stub for Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_spi_init(uint8_t spi_id, const spi_config_t *config) {
    printf("[SPI] SPI%d init stub\n", spi_id);
    return HAL_OK;
}

hal_status_t hal_spi_deinit(uint8_t spi_id) {
    return HAL_OK;
}

hal_status_t hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_data, uint8_t *rx_data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_spi_set_cs(uint8_t spi_id, uint32_t cs_pin, bool active) {
    return HAL_OK;
}
EOF

cat > platforms/pico_w/hal/pwm_hal.cpp << 'EOF'
/**
 * @file pwm_hal.cpp
 * @brief PWM HAL stub for Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_pwm_init(uint8_t pwm_id, uint32_t frequency_hz) {
    printf("[PWM] PWM%d init stub (%lu Hz)\n", pwm_id, frequency_hz);
    return HAL_OK;
}

hal_status_t hal_pwm_deinit(uint8_t pwm_id) {
    return HAL_OK;
}

hal_status_t hal_pwm_set_duty(uint8_t pwm_id, uint8_t channel, float duty_percent) {
    printf("[PWM] PWM%d Ch%d duty: %.1f%%\n", pwm_id, channel, duty_percent);
    return HAL_OK;
}

hal_status_t hal_pwm_start(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}

hal_status_t hal_pwm_stop(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}
EOF

cat > platforms/pico_w/hal/timer_hal.cpp << 'EOF'
/**
 * @file timer_hal.cpp
 * @brief Timer HAL stub for Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_timer_init(uint8_t timer_id, const timer_config_t *config) {
    printf("[TIMER] Timer%d init stub\n", timer_id);
    return HAL_OK;
}

hal_status_t hal_timer_deinit(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_start(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_stop(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_get_count(uint8_t timer_id, uint32_t *count) {
    if (count) *count = 0;
    return HAL_OK;
}

hal_status_t hal_timer_reset(uint8_t timer_id) {
    return HAL_OK;
}
EOF

echo "✅ Created all missing HAL implementations!"
echo ""
echo "Files created:"
echo "- platforms/pico_w/hal/gpio_hal.cpp (full implementation)"
echo "- platforms/pico_w/hal/adc_hal.cpp (full implementation)"
echo "- platforms/pico_w/hal/i2c_hal.cpp (stub)"
echo "- platforms/pico_w/hal/spi_hal.cpp (stub)"
echo "- platforms/pico_w/hal/pwm_hal.cpp (stub)"
echo "- platforms/pico_w/hal/timer_hal.cpp (stub)"