/**
 * @file hal_demo.h
 * @brief HAL demonstration functions
 * 
 * This file provides the interface for demonstrating HAL functionality
 * including GPIO, UART, ADC, and display operations.
 * 
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef HAL_DEMO_H
#define HAL_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run a sequence of HAL feature demonstrations
 * This function demonstrates various HAL capabilities including:
 * - GPIO operations (LED blinking)
 * - UART communication
 * - ADC readings
 * - Display operations
 */
void run_hal_demo(void);

/**
 * @brief Demonstrate GPIO operations
 * Blinks LEDs and reads button states
 */
void demo_gpio_operations(void);

/**
 * @brief Demonstrate UART operations
 * Sends test messages via UART
 */
void demo_uart_operations(void);

/**
 * @brief Demonstrate ADC operations
 * Reads from multiple ADC channels
 */
void demo_adc_operations(void);

/**
 * @brief Demonstrate display operations
 * Shows text and graphics on display
 */
void demo_display_operations(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_DEMO_H
