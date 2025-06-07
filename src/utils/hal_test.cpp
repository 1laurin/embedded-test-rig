/**
 * @file hal_test.cpp
 * @brief HAL subsystem test implementation
 */

#include "hal_test.h"
#include "hal_interface.h"
#include <stdio.h>

bool test_hal_subsystems(void) {
    printf("[TEST] Starting HAL subsystem tests...\n");
    
    uint32_t passed = 0;
    uint32_t total = 0;
    
    // Test each subsystem
    total++; if (test_gpio_subsystem()) passed++;
    total++; if (test_uart_subsystem()) passed++;
    total++; if (test_adc_subsystem()) passed++;
    total++; if (test_display_subsystem()) passed++;
    
    print_test_summary(passed, total);
    
    return (passed == total);
}

bool test_gpio_subsystem(void) {
    printf("[TEST] Testing GPIO subsystem...\n");
    
    // Test basic GPIO write operations
    if (hal_gpio_write(25, GPIO_HIGH) != HAL_OK) {
        printf("[TEST] GPIO write HIGH failed\n");
        return false;
    }
    
    if (hal_gpio_write(25, GPIO_LOW) != HAL_OK) {
        printf("[TEST] GPIO write LOW failed\n");
        return false;
    }
    
    // Test GPIO toggle
    if (hal_gpio_toggle(25) != HAL_OK) {
        printf("[TEST] GPIO toggle failed\n");
        return false;
    }
    
    printf("[TEST] GPIO subsystem: PASS\n");
    return true;
}

bool test_uart_subsystem(void) {
    printf("[TEST] Testing UART subsystem...\n");
    
    // Test basic UART transmission
    const char* test_msg = "TEST";
    if (hal_uart_transmit(0, (const uint8_t*)test_msg, 4, 100) != HAL_OK) {
        printf("[TEST] UART transmit failed\n");
        return false;
    }
    
    printf("[TEST] UART subsystem: PASS\n");
    return true;
}

bool test_adc_subsystem(void) {
    printf("[TEST] Testing ADC subsystem...\n");
    
    // Test ADC read
    uint16_t adc_value;
    if (hal_adc_read(0, &adc_value) != HAL_OK) {
        printf("[TEST] ADC read failed\n");
        return false;
    }
    
    printf("[TEST] ADC subsystem: PASS (value: %u)\n", adc_value);
    return true;
}

bool test_spi_subsystem(void) {
    printf("[TEST] Testing SPI subsystem...\n");
    // SPI test implementation would go here
    printf("[TEST] SPI subsystem: SKIP (not implemented)\n");
    return true;
}

bool test_i2c_subsystem(void) {
    printf("[TEST] Testing I2C subsystem...\n");
    // I2C test implementation would go here
    printf("[TEST] I2C subsystem: SKIP (not implemented)\n");
    return true;
}

bool test_pwm_subsystem(void) {
    printf("[TEST] Testing PWM subsystem...\n");
    // PWM test implementation would go here
    printf("[TEST] PWM subsystem: SKIP (not implemented)\n");
    return true;
}

bool test_display_subsystem(void) {
    printf("[TEST] Testing display subsystem...\n");
    
    // Test basic display operations
    if (hal_display_clear(0x000000) != HAL_OK) {
        printf("[TEST] Display clear failed\n");
        return false;
    }
    
    if (hal_display_set_pixel(0, 0, 0xFFFFFF) != HAL_OK) {
        printf("[TEST] Display set pixel failed\n");
        return false;
    }
    
    printf("[TEST] Display subsystem: PASS\n");
    return true;
}

void print_test_summary(uint32_t passed, uint32_t total) {
    printf("\n[TEST] ========== TEST SUMMARY ==========\n");
    printf("[TEST] Tests passed: %lu/%lu\n", passed, total);
    printf("[TEST] Success rate: %.1f%%\n", (float)passed * 100.0f / (float)total);
    
    if (passed == total) {
        printf("[TEST] Result: ALL TESTS PASSED ✅\n");
    } else {
        printf("[TEST] Result: SOME TESTS FAILED ❌\n");
    }
    printf("[TEST] ================================\n\n");
}
