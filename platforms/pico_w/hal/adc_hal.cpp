/**
 * @file adc_hal.cpp
 * @brief ADC Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <stdio.h>

static bool adc_initialized = false;

hal_status_t hal_adc_init(void) {
    if (adc_initialized) {
        return HAL_OK;
    }
    
    printf("[ADC] Initializing ADC subsystem...\n");
    
    adc_init();
    adc_set_temp_sensor_enabled(true);
    
    adc_initialized = true;
    printf("[ADC] ADC subsystem initialized\n");
    
    return HAL_OK;
}

hal_status_t hal_adc_config(const adc_config_t *config) {
    if (!adc_initialized || !config) {
        return HAL_ERROR;
    }
    
    if (config->channel < 5) {
        if (config->channel < 3) {
            adc_gpio_init(26 + config->channel);
        }
        return HAL_OK;
    }
    
    return HAL_INVALID_PARAM;
}

hal_status_t hal_adc_read(uint8_t channel, uint16_t *value) {
    if (!adc_initialized || !value || channel >= 5) {
        return HAL_INVALID_PARAM;
    }
    
    adc_select_input(channel);
    *value = adc_read();
    
    return HAL_OK;
}

hal_status_t hal_adc_read_voltage(uint8_t channel, float *voltage) {
    if (!voltage) {
        return HAL_INVALID_PARAM;
    }
    
    uint16_t raw_value;
    hal_status_t status = hal_adc_read(channel, &raw_value);
    
    if (status == HAL_OK) {
        *voltage = (float)raw_value * ADC_REFERENCE_VOLTAGE / (1 << ADC_RESOLUTION_BITS);
    }
    
    return status;
}

hal_status_t hal_adc_start_continuous(uint8_t channel, void (*callback)(uint8_t, uint16_t)) {
    (void)channel;
    (void)callback;
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_adc_stop_continuous(uint8_t channel) {
    (void)channel;
    return HAL_NOT_SUPPORTED;
}
