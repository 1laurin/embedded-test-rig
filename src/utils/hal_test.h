/**
 * @file hal_test.h
 * @brief HAL subsystem test utilities
 * 
 * This file provides testing functions for validating HAL subsystem
 * functionality and ensuring hardware is working correctly.
 * 
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef HAL_TEST_H
#define HAL_TEST_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test all HAL subsystems and report pass/fail
 * @return true if all tests passed, false otherwise
 */
bool test_hal_subsystems(void);

/**
 * @brief Test GPIO subsystem
 * @return true if GPIO tests passed, false otherwise
 */
bool test_gpio_subsystem(void);

/**
 * @brief Test UART subsystem
 * @return true if UART tests passed, false otherwise
 */
bool test_uart_subsystem(void);

/**
 * @brief Test ADC subsystem
 * @return true if ADC tests passed, false otherwise
 */
bool test_adc_subsystem(void);

/**
 * @brief Test SPI subsystem
 * @return true if SPI tests passed, false otherwise
 */
bool test_spi_subsystem(void);

/**
 * @brief Test I2C subsystem
 * @return true if I2C tests passed, false otherwise
 */
bool test_i2c_subsystem(void);

/**
 * @brief Test PWM subsystem
 * @return true if PWM tests passed, false otherwise
 */
bool test_pwm_subsystem(void);

/**
 * @brief Test display subsystem
 * @return true if display tests passed, false otherwise
 */
bool test_display_subsystem(void);

/**
 * @brief Print test results summary
 * @param passed Number of tests that passed
 * @param total Total number of tests run
 */
void print_test_summary(uint32_t passed, uint32_t total);

#ifdef __cplusplus
}
#endif

#endif // HAL_TEST_H
