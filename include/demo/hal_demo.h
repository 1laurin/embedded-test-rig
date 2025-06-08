/**
 * @file hal_demo.h
 * @brief HAL demonstration functions
 */

#ifndef HAL_DEMO_H
#define HAL_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

void run_hal_demo(void);
void demo_gpio_operations(void);
void demo_uart_operations(void);
void demo_adc_operations(void);
void demo_display_operations(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_DEMO_H
