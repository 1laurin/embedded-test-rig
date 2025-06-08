/**
 * @file uart_hal.cpp
 * @brief UART Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../utils/hal_interface.h"
#include "../include/board_config.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

#define MAX_UART_INSTANCES 2

typedef struct {
    uart_inst_t *instance;
    bool initialized;
    uint32_t baudrate;
} uart_context_t;

static uart_context_t uart_contexts[MAX_UART_INSTANCES] = {{0}};

static uart_inst_t *get_uart_instance(uint8_t uart_id) {
    switch (uart_id) {
        case 0: return uart0;
        case 1: return uart1;
        default: return NULL;
    }
}

hal_status_t hal_uart_init(uint8_t uart_id, const uart_config_t *config) {
    if (uart_id >= MAX_UART_INSTANCES || !config) {
        return HAL_INVALID_PARAM;
    }
    
    uart_inst_t *uart = get_uart_instance(uart_id);
    if (!uart) {
        return HAL_ERROR;
    }
    
    uint actual_baud = uart_init(uart, config->baudrate);
    if (actual_baud == 0) {
        return HAL_INIT_FAILED;
    }
    
    // Configure GPIO pins
    if (uart_id == 0) {
        gpio_set_function(UART_DEBUG_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART_DEBUG_RX_PIN, GPIO_FUNC_UART);
    } else {
        gpio_set_function(UART_EXT_TX_PIN, GPIO_FUNC_UART);
        gpio_set_function(UART_EXT_RX_PIN, GPIO_FUNC_UART);
    }
    
    uart_contexts[uart_id].instance = uart;
    uart_contexts[uart_id].initialized = true;
    uart_contexts[uart_id].baudrate = actual_baud;
    
    printf("[UART] UART%d initialized at %lu baud\n", uart_id, actual_baud);
    
    return HAL_OK;
}

hal_status_t hal_uart_deinit(uint8_t uart_id) {
    if (uart_id >= MAX_UART_INSTANCES) {
        return HAL_INVALID_PARAM;
    }
    
    if (uart_contexts[uart_id].initialized) {
        uart_deinit(uart_contexts[uart_id].instance);
        uart_contexts[uart_id].initialized = false;
    }
    
    return HAL_OK;
}

hal_status_t hal_uart_transmit(uint8_t uart_id, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    if (uart_id >= MAX_UART_INSTANCES || !data || size == 0) {
        return HAL_INVALID_PARAM;
    }
    
    if (!uart_contexts[uart_id].initialized) {
        return HAL_ERROR;
    }
    
    uart_inst_t *uart = uart_contexts[uart_id].instance;
    
    for (size_t i = 0; i < size; i++) {
        uart_putc_raw(uart, data[i]);
    }
    
    return HAL_OK;
}

hal_status_t hal_uart_receive(uint8_t uart_id, uint8_t *data, size_t size, size_t *received, uint32_t timeout_ms) {
    if (uart_id >= MAX_UART_INSTANCES || !data || !received) {
        return HAL_INVALID_PARAM;
    }
    
    if (!uart_contexts[uart_id].initialized) {
        return HAL_ERROR;
    }
    
    uart_inst_t *uart = uart_contexts[uart_id].instance;
    *received = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (uart_is_readable(uart)) {
            data[i] = uart_getc(uart);
            (*received)++;
        } else {
            break;
        }
    }
    
    return HAL_OK;
}

hal_status_t hal_uart_available(uint8_t uart_id, size_t *available) {
    if (uart_id >= MAX_UART_INSTANCES || !available) {
        return HAL_INVALID_PARAM;
    }
    
    if (!uart_contexts[uart_id].initialized) {
        return HAL_ERROR;
    }
    
    *available = uart_is_readable(uart_contexts[uart_id].instance) ? 1 : 0;
    return HAL_OK;
}

hal_status_t hal_uart_flush(uint8_t uart_id) {
    if (uart_id >= MAX_UART_INSTANCES) {
        return HAL_INVALID_PARAM;
    }
    
    if (!uart_contexts[uart_id].initialized) {
        return HAL_ERROR;
    }
    
    // Pico SDK doesn't have explicit flush, just wait a bit
    sleep_ms(10);
    return HAL_OK;
}
