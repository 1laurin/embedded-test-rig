/**
 * @file hal_demo.cpp
 * @brief HAL demonstration implementation
 */

#include "../utils/hal_demo.h"
#include "../utils/hal_interface.h"
#include <stdio.h>
#include <string.h>  // Added for strlen

void run_hal_demo(void) {
    printf("[DEMO] Starting HAL demonstration...\n");
    
    demo_gpio_operations();
    demo_uart_operations();
    demo_adc_operations();
    demo_display_operations();
    
    printf("[DEMO] HAL demonstration complete!\n");
}

void demo_gpio_operations(void) {
    printf("[DEMO] Testing GPIO operations...\n");
    
    // Blink built-in LED
    for (int i = 0; i < 3; i++) {
        hal_gpio_write(25, GPIO_HIGH);  // Pico built-in LED
        hal_delay_ms(200);
        hal_gpio_write(25, GPIO_LOW);
        hal_delay_ms(200);
    }
    
    printf("[DEMO] GPIO test complete\n");
}

void demo_uart_operations(void) {
    printf("[DEMO] Testing UART operations...\n");
    
    // Test message
    const char* test_msg = "HAL UART Test Message\r\n";
    
    // Try to send via UART 1 (external)
    hal_uart_transmit(1, (const uint8_t*)test_msg, strlen(test_msg), 1000);
    
    printf("[DEMO] UART test complete\n");
}

void demo_adc_operations(void) {
    printf("[DEMO] Testing ADC operations...\n");
    
    // Read a few ADC channels
    for (int ch = 0; ch < 3; ch++) {
        uint16_t adc_value;
        if (hal_adc_read(ch, &adc_value) == HAL_OK) {
            float voltage = (float)adc_value * 3.3f / 4096.0f;
            printf("[DEMO] ADC Ch%d: %u counts (%.3f V)\n", ch, adc_value, voltage);
        }
    }
    
    printf("[DEMO] ADC test complete\n");
}

void demo_display_operations(void) {
    printf("[DEMO] Testing display operations...\n");
    
    // Basic display test
    hal_display_clear(0x000000);
    hal_display_draw_text(10, 10, "HAL Demo", 0xFFFFFF, 0x000000);
    hal_display_draw_rect(50, 50, 100, 60, 0xFF0000, false);
    hal_display_flush();
    
    printf("[DEMO] Display test complete\n");
}
