/**
 * @file hal_test.h
 * @brief HAL subsystem test utilities
 */

#ifndef HAL_TEST_H
#define HAL_TEST_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool test_hal_subsystems(void);
bool test_gpio_subsystem(void);
bool test_uart_subsystem(void);
bool test_adc_subsystem(void);
bool test_spi_subsystem(void);
bool test_i2c_subsystem(void);
bool test_pwm_subsystem(void);
bool test_display_subsystem(void);
void print_test_summary(uint32_t passed, uint32_t total);

#ifdef __cplusplus
}
#endif

#endif // HAL_TEST_H
