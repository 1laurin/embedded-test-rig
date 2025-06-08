/**
 * @file hal_interface.h
 * @brief Hardware Abstraction Layer Interface for Multi-Channel Diagnostic Test Rig
 *
 * This file defines the abstract interface that all platform-specific HAL
 * implementations must follow. It provides a uniform API for hardware access
 * across different microcontroller platforms (Pico W, STM32, etc.).
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // TYPE DEFINITIONS
    // =============================================================================

    /**
     * @brief HAL return status codes
     */
    typedef enum
    {
        HAL_OK = 0,
        HAL_ERROR = 1,
        HAL_BUSY = 2,
        HAL_TIMEOUT = 3,
        HAL_INVALID_PARAM = 4,
        HAL_NOT_SUPPORTED = 5,
        HAL_INIT_FAILED = 6
    } hal_status_t;

    /**
     * @brief GPIO pin states
     */
    typedef enum
    {
        GPIO_LOW = 0,
        GPIO_HIGH = 1
    } gpio_state_t;

    /**
     * @brief GPIO pin modes
     */
    typedef enum
    {
        GPIO_INPUT = 0,
        GPIO_OUTPUT = 1,
        GPIO_INPUT_PULLUP = 2,
        GPIO_INPUT_PULLDOWN = 3,
        GPIO_OPEN_DRAIN = 4
    } gpio_mode_t;

    /**
     * @brief ADC channel configuration
     */
    typedef struct
    {
        uint8_t channel;
        uint16_t resolution_bits;
        float reference_voltage;
        uint32_t sample_time_us;
    } adc_config_t;

    /**
     * @brief UART configuration
     */
    typedef struct
    {
        uint32_t baudrate;
        uint8_t data_bits;
        uint8_t stop_bits;
        uint8_t parity; // 0=none, 1=odd, 2=even
        bool flow_control;
    } uart_config_t;

    /**
     * @brief SPI configuration
     */
    typedef struct
    {
        uint32_t frequency;
        uint8_t mode; // 0-3 (CPOL/CPHA combinations)
        uint8_t data_bits;
        bool msb_first;
    } spi_config_t;

    /**
     * @brief I2C configuration
     */
    typedef struct
    {
        uint32_t frequency;
        uint8_t address_bits; // 7 or 10
        bool fast_mode;
    } i2c_config_t;

    /**
     * @brief Display buffer structure
     */
    typedef struct
    {
        uint16_t width;
        uint16_t height;
        uint16_t x_offset;
        uint16_t y_offset;
        const uint8_t *data;
        size_t data_size;
    } display_buffer_t;

    /**
     * @brief Timer configuration
     */
    typedef struct
    {
        uint32_t frequency_hz;
        bool auto_reload;
        bool interrupt_enable;
        void (*callback)(void);
    } timer_config_t;

    // =============================================================================
    // SYSTEM FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize the HAL layer
     * @return HAL status code
     */
    hal_status_t hal_init(void);

    /**
     * @brief Deinitialize the HAL layer
     * @return HAL status code
     */
    hal_status_t hal_deinit(void);

    /**
     * @brief Get system tick count in milliseconds
     * @return Current tick count
     */
    uint32_t hal_get_tick_ms(void);

    /**
     * @brief Delay execution for specified milliseconds
     * @param ms Delay time in milliseconds
     */
    void hal_delay_ms(uint32_t ms);

    /**
     * @brief Delay execution for specified microseconds
     * @param us Delay time in microseconds
     */
    void hal_delay_us(uint32_t us);

    /**
     * @brief Reset the system
     */
    void hal_system_reset(void);

    // =============================================================================
    // GPIO FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize GPIO subsystem
     * @return HAL status code
     */
    hal_status_t hal_gpio_init(void);

    /**
     * @brief Configure a GPIO pin
     * @param pin Pin number (platform-specific)
     * @param mode GPIO mode
     * @return HAL status code
     */
    hal_status_t hal_gpio_config(uint32_t pin, gpio_mode_t mode);

    /**
     * @brief Write to a GPIO pin
     * @param pin Pin number
     * @param state Pin state (HIGH/LOW)
     * @return HAL status code
     */
    hal_status_t hal_gpio_write(uint32_t pin, gpio_state_t state);

    /**
     * @brief Read from a GPIO pin
     * @param pin Pin number
     * @param state Pointer to store pin state
     * @return HAL status code
     */
    hal_status_t hal_gpio_read(uint32_t pin, gpio_state_t *state);

    /**
     * @brief Toggle a GPIO pin
     * @param pin Pin number
     * @return HAL status code
     */
    hal_status_t hal_gpio_toggle(uint32_t pin);

    /**
     * @brief Enable GPIO interrupt
     * @param pin Pin number
     * @param trigger_edge Edge type (rising=1, falling=2, both=3)
     * @param callback Interrupt callback function
     * @return HAL status code
     */
    hal_status_t hal_gpio_interrupt_enable(uint32_t pin, uint8_t trigger_edge, void (*callback)(uint32_t pin));

    /**
     * @brief Disable GPIO interrupt
     * @param pin Pin number
     * @return HAL status code
     */
    hal_status_t hal_gpio_interrupt_disable(uint32_t pin);

    // =============================================================================
    // ADC FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize ADC subsystem
     * @return HAL status code
     */
    hal_status_t hal_adc_init(void);

    /**
     * @brief Configure ADC channel
     * @param config ADC configuration structure
     * @return HAL status code
     */
    hal_status_t hal_adc_config(const adc_config_t *config);

    /**
     * @brief Read ADC value (blocking)
     * @param channel ADC channel number
     * @param value Pointer to store ADC reading
     * @return HAL status code
     */
    hal_status_t hal_adc_read(uint8_t channel, uint16_t *value);

    /**
     * @brief Read ADC value as voltage
     * @param channel ADC channel number
     * @param voltage Pointer to store voltage reading
     * @return HAL status code
     */
    hal_status_t hal_adc_read_voltage(uint8_t channel, float *voltage);

    /**
     * @brief Start continuous ADC conversion
     * @param channel ADC channel number
     * @param callback Conversion complete callback
     * @return HAL status code
     */
    hal_status_t hal_adc_start_continuous(uint8_t channel, void (*callback)(uint8_t channel, uint16_t value));

    /**
     * @brief Stop continuous ADC conversion
     * @param channel ADC channel number
     * @return HAL status code
     */
    hal_status_t hal_adc_stop_continuous(uint8_t channel);

    // =============================================================================
    // UART FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize UART subsystem
     * @param uart_id UART instance ID
     * @param config UART configuration
     * @return HAL status code
     */
    hal_status_t hal_uart_init(uint8_t uart_id, const uart_config_t *config);

    /**
     * @brief Deinitialize UART instance
     * @param uart_id UART instance ID
     * @return HAL status code
     */
    hal_status_t hal_uart_deinit(uint8_t uart_id);

    /**
     * @brief Transmit data via UART (blocking)
     * @param uart_id UART instance ID
     * @param data Data buffer to transmit
     * @param size Number of bytes to transmit
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_uart_transmit(uint8_t uart_id, const uint8_t *data, size_t size, uint32_t timeout_ms);

    /**
     * @brief Receive data via UART (blocking)
     * @param uart_id UART instance ID
     * @param data Buffer to store received data
     * @param size Maximum number of bytes to receive
     * @param received Pointer to store actual bytes received
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_uart_receive(uint8_t uart_id, uint8_t *data, size_t size, size_t *received, uint32_t timeout_ms);

    /**
     * @brief Check if UART data is available
     * @param uart_id UART instance ID
     * @param available Pointer to store number of available bytes
     * @return HAL status code
     */
    hal_status_t hal_uart_available(uint8_t uart_id, size_t *available);

    /**
     * @brief Flush UART buffers
     * @param uart_id UART instance ID
     * @return HAL status code
     */
    hal_status_t hal_uart_flush(uint8_t uart_id);

    // =============================================================================
    // SPI FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize SPI subsystem
     * @param spi_id SPI instance ID
     * @param config SPI configuration
     * @return HAL status code
     */
    hal_status_t hal_spi_init(uint8_t spi_id, const spi_config_t *config);

    /**
     * @brief Deinitialize SPI instance
     * @param spi_id SPI instance ID
     * @return HAL status code
     */
    hal_status_t hal_spi_deinit(uint8_t spi_id);

    /**
     * @brief Transmit data via SPI
     * @param spi_id SPI instance ID
     * @param tx_data Data to transmit
     * @param rx_data Buffer for received data (can be NULL)
     * @param size Number of bytes to transfer
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_data, uint8_t *rx_data, size_t size, uint32_t timeout_ms);

    /**
     * @brief Set SPI chip select state
     * @param spi_id SPI instance ID
     * @param cs_pin Chip select pin
     * @param active Active state (true = assert CS)
     * @return HAL status code
     */
    hal_status_t hal_spi_set_cs(uint8_t spi_id, uint32_t cs_pin, bool active);

    // =============================================================================
    // I2C FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize I2C subsystem
     * @param i2c_id I2C instance ID
     * @param config I2C configuration
     * @return HAL status code
     */
    hal_status_t hal_i2c_init(uint8_t i2c_id, const i2c_config_t *config);

    /**
     * @brief Deinitialize I2C instance
     * @param i2c_id I2C instance ID
     * @return HAL status code
     */
    hal_status_t hal_i2c_deinit(uint8_t i2c_id);

    /**
     * @brief Transmit data to I2C device
     * @param i2c_id I2C instance ID
     * @param device_addr I2C device address
     * @param data Data to transmit
     * @param size Number of bytes to transmit
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_i2c_transmit(uint8_t i2c_id, uint8_t device_addr, const uint8_t *data, size_t size, uint32_t timeout_ms);

    /**
     * @brief Receive data from I2C device
     * @param i2c_id I2C instance ID
     * @param device_addr I2C device address
     * @param data Buffer to store received data
     * @param size Number of bytes to receive
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_i2c_receive(uint8_t i2c_id, uint8_t device_addr, uint8_t *data, size_t size, uint32_t timeout_ms);

    /**
     * @brief Write to I2C device register
     * @param i2c_id I2C instance ID
     * @param device_addr I2C device address
     * @param reg_addr Register address
     * @param data Data to write
     * @param size Number of bytes to write
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_i2c_write_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, size_t size, uint32_t timeout_ms);

    /**
     * @brief Read from I2C device register
     * @param i2c_id I2C instance ID
     * @param device_addr I2C device address
     * @param reg_addr Register address
     * @param data Buffer to store read data
     * @param size Number of bytes to read
     * @param timeout_ms Timeout in milliseconds
     * @return HAL status code
     */
    hal_status_t hal_i2c_read_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t size, uint32_t timeout_ms);

    // =============================================================================
    // DISPLAY FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize display subsystem
     * @return HAL status code
     */
    hal_status_t hal_display_init(void);

    /**
     * @brief Deinitialize display subsystem
     * @return HAL status code
     */
    hal_status_t hal_display_deinit(void);

    /**
     * @brief Clear display
     * @param color Clear color (platform-specific format)
     * @return HAL status code
     */
    hal_status_t hal_display_clear(uint32_t color);

    /**
     * @brief Update display with buffer data
     * @param buffer Display buffer structure
     * @return HAL status code
     */
    hal_status_t hal_display_update(const display_buffer_t *buffer);

    /**
     * @brief Set display pixel
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Pixel color
     * @return HAL status code
     */
    hal_status_t hal_display_set_pixel(uint16_t x, uint16_t y, uint32_t color);

    /**
     * @brief Draw rectangle on display
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Rectangle width
     * @param height Rectangle height
     * @param color Rectangle color
     * @param filled Fill rectangle if true
     * @return HAL status code
     */
    hal_status_t hal_display_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, bool filled);

    /**
     * @brief Draw text on display
     * @param x X coordinate
     * @param y Y coordinate
     * @param text Text string
     * @param color Text color
     * @param bg_color Background color
     * @return HAL status code
     */
    hal_status_t hal_display_draw_text(uint16_t x, uint16_t y, const char *text, uint32_t color, uint32_t bg_color);

    /**
     * @brief Set display brightness
     * @param brightness Brightness level (0-100)
     * @return HAL status code
     */
    hal_status_t hal_display_set_brightness(uint8_t brightness);

    /**
     * @brief Flush display buffer to screen
     * @return HAL status code
     */
    hal_status_t hal_display_flush(void);

    // =============================================================================
    // TIMER FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize timer subsystem
     * @param timer_id Timer instance ID
     * @param config Timer configuration
     * @return HAL status code
     */
    hal_status_t hal_timer_init(uint8_t timer_id, const timer_config_t *config);

    /**
     * @brief Deinitialize timer
     * @param timer_id Timer instance ID
     * @return HAL status code
     */
    hal_status_t hal_timer_deinit(uint8_t timer_id);

    /**
     * @brief Start timer
     * @param timer_id Timer instance ID
     * @return HAL status code
     */
    hal_status_t hal_timer_start(uint8_t timer_id);

    /**
     * @brief Stop timer
     * @param timer_id Timer instance ID
     * @return HAL status code
     */
    hal_status_t hal_timer_stop(uint8_t timer_id);

    /**
     * @brief Get timer count
     * @param timer_id Timer instance ID
     * @param count Pointer to store timer count
     * @return HAL status code
     */
    hal_status_t hal_timer_get_count(uint8_t timer_id, uint32_t *count);

    /**
     * @brief Reset timer count
     * @param timer_id Timer instance ID
     * @return HAL status code
     */
    hal_status_t hal_timer_reset(uint8_t timer_id);

    // =============================================================================
    // PWM FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize PWM subsystem
     * @param pwm_id PWM instance ID
     * @param frequency_hz PWM frequency in Hz
     * @return HAL status code
     */
    hal_status_t hal_pwm_init(uint8_t pwm_id, uint32_t frequency_hz);

    /**
     * @brief Deinitialize PWM
     * @param pwm_id PWM instance ID
     * @return HAL status code
     */
    hal_status_t hal_pwm_deinit(uint8_t pwm_id);

    /**
     * @brief Set PWM duty cycle
     * @param pwm_id PWM instance ID
     * @param channel PWM channel
     * @param duty_percent Duty cycle percentage (0-100)
     * @return HAL status code
     */
    hal_status_t hal_pwm_set_duty(uint8_t pwm_id, uint8_t channel, float duty_percent);

    /**
     * @brief Start PWM output
     * @param pwm_id PWM instance ID
     * @param channel PWM channel
     * @return HAL status code
     */
    hal_status_t hal_pwm_start(uint8_t pwm_id, uint8_t channel);

    /**
     * @brief Stop PWM output
     * @param pwm_id PWM instance ID
     * @param channel PWM channel
     * @return HAL status code
     */
    hal_status_t hal_pwm_stop(uint8_t pwm_id, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif // HAL_INTERFACE_H