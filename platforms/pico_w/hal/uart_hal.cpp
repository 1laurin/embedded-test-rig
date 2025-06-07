/**
 * @file uart_hal.cpp
 * @brief UART Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 *
 * This file implements the UART HAL interface for the Pico W platform using
 * the Pico SDK UART functions.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include <stdio.h>
#include <string.h>
#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// =============================================================================
// PRIVATE CONSTANTS
// =============================================================================

#define MAX_UART_INSTANCES 2
#define UART_TIMEOUT_US 1000000 // 1 second timeout in microseconds

// =============================================================================
// PRIVATE TYPES
// =============================================================================

typedef struct
{
    uart_inst_t *instance;
    bool initialized;
    uint32_t baudrate;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
    uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t tx_head;
    volatile uint16_t tx_tail;
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
    volatile uint16_t rx_count;
} uart_context_t;

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static uart_context_t uart_contexts[MAX_UART_INSTANCES] = {0};
static bool uart_subsystem_initialized = false;

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static uart_context_t *get_uart_context(uint8_t uart_id);
static uart_inst_t *get_uart_instance(uint8_t uart_id);
static void uart_irq_handler(void);
static void uart0_irq_handler(void);
static void uart1_irq_handler(void);
static hal_status_t configure_uart_pins(uint8_t uart_id, uint8_t tx_pin, uint8_t rx_pin);
static void uart_put_char_buffered(uart_context_t *ctx, uint8_t c);
static bool uart_get_char_buffered(uart_context_t *ctx, uint8_t *c);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize UART subsystem
 * @param uart_id UART instance ID (0 or 1)
 * @param config UART configuration
 * @return HAL status code
 */
hal_status_t hal_uart_init(uint8_t uart_id, const uart_config_t *config)
{
    if (uart_id >= MAX_UART_INSTANCES || config == nullptr)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr)
    {
        return HAL_ERROR;
    }

    // Don't re-initialize if already done
    if (ctx->initialized)
    {
        return HAL_OK;
    }

    // Get UART instance
    uart_inst_t *uart = get_uart_instance(uart_id);
    if (uart == nullptr)
    {
        return HAL_ERROR;
    }

    // Initialize UART with specified baud rate
    uint actual_baud = uart_init(uart, config->baudrate);
    if (actual_baud == 0)
    {
        return HAL_INIT_FAILED;
    }

    // Configure data format
    uart_data_bits_t data_bits;
    switch (config->data_bits)
    {
    case 5:
        data_bits = 5;
        break;
    case 6:
        data_bits = 6;
        break;
    case 7:
        data_bits = 7;
        break;
    case 8:
        data_bits = 8;
        break;
    default:
        return HAL_INVALID_PARAM;
    }

    uart_stop_bits_t stop_bits;
    switch (config->stop_bits)
    {
    case 1:
        stop_bits = 1;
        break;
    case 2:
        stop_bits = 2;
        break;
    default:
        return HAL_INVALID_PARAM;
    }

    uart_parity_t parity;
    switch (config->parity)
    {
    case 0:
        parity = UART_PARITY_NONE;
        break;
    case 1:
        parity = UART_PARITY_ODD;
        break;
    case 2:
        parity = UART_PARITY_EVEN;
        break;
    default:
        return HAL_INVALID_PARAM;
    }

    uart_set_format(uart, data_bits, stop_bits, parity);

    // Configure GPIO pins
    hal_status_t pin_status = HAL_OK;
    if (uart_id == 0)
    {
        pin_status = configure_uart_pins(uart_id, UART_DEBUG_TX_PIN, UART_DEBUG_RX_PIN);
        ctx->tx_pin = UART_DEBUG_TX_PIN;
        ctx->rx_pin = UART_DEBUG_RX_PIN;
    }
    else if (uart_id == 1)
    {
        pin_status = configure_uart_pins(uart_id, UART_EXT_TX_PIN, UART_EXT_RX_PIN);
        ctx->tx_pin = UART_EXT_TX_PIN;
        ctx->rx_pin = UART_EXT_RX_PIN;
    }

    if (pin_status != HAL_OK)
    {
        return pin_status;
    }

    // Enable FIFO
    uart_set_fifo_enabled(uart, true);

    // Initialize context
    ctx->instance = uart;
    ctx->baudrate = actual_baud;
    ctx->tx_head = 0;
    ctx->tx_tail = 0;
    ctx->rx_head = 0;
    ctx->rx_tail = 0;
    ctx->rx_count = 0;

    // Set up interrupts
    int uart_irq = (uart_id == 0) ? UART0_IRQ : UART1_IRQ;

    // Enable UART interrupt for RX
    uart_set_irq_enables(uart, true, false); // Enable RX, disable TX for now

    // Set interrupt handler
    if (uart_id == 0)
    {
        irq_set_exclusive_handler(UART0_IRQ, uart0_irq_handler);
        irq_set_enabled(UART0_IRQ, true);
    }
    else
    {
        irq_set_exclusive_handler(UART1_IRQ, uart1_irq_handler);
        irq_set_enabled(UART1_IRQ, true);
    }

    ctx->initialized = true;
    uart_subsystem_initialized = true;

    printf("[UART] UART%d initialized at %u baud\n", uart_id, actual_baud);

    return HAL_OK;
}

/**
 * @brief Deinitialize UART instance
 * @param uart_id UART instance ID
 * @return HAL status code
 */
hal_status_t hal_uart_deinit(uint8_t uart_id)
{
    if (uart_id >= MAX_UART_INSTANCES)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr || !ctx->initialized)
    {
        return HAL_ERROR;
    }

    // Disable interrupts
    int uart_irq = (uart_id == 0) ? UART0_IRQ : UART1_IRQ;
    irq_set_enabled(uart_irq, false);

    // Disable UART
    uart_deinit(ctx->instance);

    // Reset context
    memset(ctx, 0, sizeof(uart_context_t));

    printf("[UART] UART%d deinitialized\n", uart_id);

    return HAL_OK;
}

/**
 * @brief Transmit data via UART (blocking)
 * @param uart_id UART instance ID
 * @param data Data buffer to transmit
 * @param size Number of bytes to transmit
 * @param timeout_ms Timeout in milliseconds
 * @return HAL status code
 */
hal_status_t hal_uart_transmit(uint8_t uart_id, const uint8_t *data, size_t size, uint32_t timeout_ms)
{
    if (uart_id >= MAX_UART_INSTANCES || data == nullptr || size == 0)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr || !ctx->initialized)
    {
        return HAL_ERROR;
    }

    uint64_t start_time = time_us_64();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;

    for (size_t i = 0; i < size; i++)
    {
        // Check for timeout
        if (timeout_ms > 0 && (time_us_64() - start_time) > timeout_us)
        {
            return HAL_TIMEOUT;
        }

        // Wait for TX FIFO to have space (blocking)
        while (!uart_is_writable(ctx->instance))
        {
            if (timeout_ms > 0 && (time_us_64() - start_time) > timeout_us)
            {
                return HAL_TIMEOUT;
            }
            tight_loop_contents();
        }

        uart_putc_raw(ctx->instance, data[i]);
    }

    return HAL_OK;
}

/**
 * @brief Receive data via UART (blocking)
 * @param uart_id UART instance ID
 * @param data Buffer to store received data
 * @param size Maximum number of bytes to receive
 * @param received Pointer to store actual bytes received
 * @param timeout_ms Timeout in milliseconds
 * @return HAL status code
 */
hal_status_t hal_uart_receive(uint8_t uart_id, uint8_t *data, size_t size, size_t *received, uint32_t timeout_ms)
{
    if (uart_id >= MAX_UART_INSTANCES || data == nullptr || size == 0 || received == nullptr)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr || !ctx->initialized)
    {
        return HAL_ERROR;
    }

    *received = 0;
    uint64_t start_time = time_us_64();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t byte;

        // Try to get byte from buffer first
        if (uart_get_char_buffered(ctx, &byte))
        {
            data[i] = byte;
            (*received)++;
            continue;
        }

        // If no buffered data, wait for new data
        uint64_t byte_start_time = time_us_64();
        while (!uart_is_readable(ctx->instance))
        {
            if (timeout_ms > 0 && (time_us_64() - start_time) > timeout_us)
            {
                return (*received > 0) ? HAL_OK : HAL_TIMEOUT;
            }
            tight_loop_contents();
        }

        // Read byte directly from UART
        data[i] = uart_getc(ctx->instance);
        (*received)++;
    }

    return HAL_OK;
}

/**
 * @brief Check if UART data is available
 * @param uart_id UART instance ID
 * @param available Pointer to store number of available bytes
 * @return HAL status code
 */
hal_status_t hal_uart_available(uint8_t uart_id, size_t *available)
{
    if (uart_id >= MAX_UART_INSTANCES || available == nullptr)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr || !ctx->initialized)
    {
        return HAL_ERROR;
    }

    // Count buffered bytes plus any in UART FIFO
    *available = ctx->rx_count;

    // Add any bytes in the UART RX FIFO
    while (uart_is_readable(ctx->instance))
    {
        (*available)++;
        // We could read and buffer these, but for simplicity just count them
        break; // Just check if any are available
    }

    return HAL_OK;
}

/**
 * @brief Flush UART buffers
 * @param uart_id UART instance ID
 * @return HAL status code
 */
hal_status_t hal_uart_flush(uint8_t uart_id)
{
    if (uart_id >= MAX_UART_INSTANCES)
    {
        return HAL_INVALID_PARAM;
    }

    uart_context_t *ctx = get_uart_context(uart_id);
    if (ctx == nullptr || !ctx->initialized)
    {
        return HAL_ERROR;
    }

    // Wait for TX FIFO to empty
    while (!uart_is_writable(ctx->instance))
    {
        tight_loop_contents();
    }

    // Clear RX buffer
    ctx->rx_head = 0;
    ctx->rx_tail = 0;
    ctx->rx_count = 0;

    // Clear any remaining data in RX FIFO
    while (uart_is_readable(ctx->instance))
    {
        uart_getc(ctx->instance);
    }

    return HAL_OK;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Get UART context by ID
 */
static uart_context_t *get_uart_context(uint8_t uart_id)
{
    if (uart_id >= MAX_UART_INSTANCES)
    {
        return nullptr;
    }
    return &uart_contexts[uart_id];
}

/**
 * @brief Get UART instance by ID
 */
static uart_inst_t *get_uart_instance(uint8_t uart_id)
{
    switch (uart_id)
    {
    case 0:
        return uart0;
    case 1:
        return uart1;
    default:
        return nullptr;
    }
}

/**
 * @brief Configure UART GPIO pins
 */
static hal_status_t configure_uart_pins(uint8_t uart_id, uint8_t tx_pin, uint8_t rx_pin)
{
    // Set GPIO functions for UART
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);

    return HAL_OK;
}

/**
 * @brief UART0 interrupt handler
 */
static void uart0_irq_handler(void)
{
    uart_context_t *ctx = &uart_contexts[0];

    // Handle RX interrupt
    while (uart_is_readable(ctx->instance))
    {
        uint8_t byte = uart_getc(ctx->instance);
        uart_put_char_buffered(ctx, byte);
    }
}

/**
 * @brief UART1 interrupt handler
 */
static void uart1_irq_handler(void)
{
    uart_context_t *ctx = &uart_contexts[1];

    // Handle RX interrupt
    while (uart_is_readable(ctx->instance))
    {
        uint8_t byte = uart_getc(ctx->instance);
        uart_put_char_buffered(ctx, byte);
    }
}

/**
 * @brief Put character in RX buffer
 */
static void uart_put_char_buffered(uart_context_t *ctx, uint8_t c)
{
    if (ctx->rx_count < UART_RX_BUFFER_SIZE)
    {
        ctx->rx_buffer[ctx->rx_head] = c;
        ctx->rx_head = (ctx->rx_head + 1) % UART_RX_BUFFER_SIZE;
        ctx->rx_count++;
    }
    // If buffer is full, drop the character (could implement overwrite policy)
}

/**
 * @brief Get character from RX buffer
 */
static bool uart_get_char_buffered(uart_context_t *ctx, uint8_t *c)
{
    if (ctx->rx_count == 0)
    {
        return false;
    }

    *c = ctx->rx_buffer[ctx->rx_tail];
    ctx->rx_tail = (ctx->rx_tail + 1) % UART_RX_BUFFER_SIZE;
    ctx->rx_count--;

    return true;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Print string to debug UART
 * @param str String to print
 */
void pico_uart_print_debug(const char *str)
{
    if (uart_contexts[0].initialized)
    {
        hal_uart_transmit(0, (const uint8_t *)str, strlen(str), 1000);
    }
}

/**
 * @brief Print formatted string to debug UART
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void pico_uart_printf_debug(const char *format, ...)
{
    if (!uart_contexts[0].initialized)
    {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        hal_uart_transmit(0, (const uint8_t *)buffer, len, 1000);
    }
}

/**
 * @brief Send single byte to UART
 * @param uart_id UART instance ID
 * @param byte Byte to send
 * @return HAL status code
 */
hal_status_t pico_uart_send_byte(uint8_t uart_id, uint8_t byte)
{
    return hal_uart_transmit(uart_id, &byte, 1, 100);
}

/**
 * @brief Receive single byte from UART with timeout
 * @param uart_id UART instance ID
 * @param byte Pointer to store received byte
 * @param timeout_ms Timeout in milliseconds
 * @return HAL status code
 */
hal_status_t pico_uart_receive_byte(uint8_t uart_id, uint8_t *byte, uint32_t timeout_ms)
{
    size_t received;
    return hal_uart_receive(uart_id, byte, 1, &received, timeout_ms);
}