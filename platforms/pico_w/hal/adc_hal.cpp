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

#include "hal_interface.h"
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
