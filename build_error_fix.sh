#!/bin/bash

# Build Error Fix Script
# Fixes the specific compilation errors found in the build output

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Fixing Build Errors ===${NC}"

# 1. Fix the include path issue in system_init.h
echo -e "${YELLOW}1. Fixing include path in system_init.h...${NC}"
SYSTEM_INIT_H="$PROJECT_ROOT/src/core/system_init.h"
if [ -f "$SYSTEM_INIT_H" ]; then
    # Fix the problematic include path
    sed -i.bak 's|#include "platforms/common/hal_interface.h"|#include "../../platforms/common/hal_interface.h"|g' "$SYSTEM_INIT_H"
    echo -e "${GREEN}✓ Fixed include path in system_init.h${NC}"
else
    echo -e "${RED}✗ system_init.h not found at $SYSTEM_INIT_H${NC}"
fi

# 2. Fix the unterminated comment in adc_hal.cpp
echo -e "${YELLOW}2. Fixing unterminated comment in adc_hal.cpp...${NC}"
ADC_HAL="$PROJECT_ROOT/platforms/pico_w/hal/adc_hal.cpp"
if [ -f "$ADC_HAL" ]; then
    # Complete the unterminated comment
    cat > "$ADC_HAL" << 'EOF'
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
EOF
    echo -e "${GREEN}✓ Fixed adc_hal.cpp${NC}"
else
    echo -e "${RED}✗ adc_hal.cpp not found${NC}"
fi

# 3. Fix the gpio_hal.cpp which contains UART code incorrectly
echo -e "${YELLOW}3. Creating proper GPIO HAL implementation...${NC}"
GPIO_HAL="$PROJECT_ROOT/platforms/pico_w/hal/gpio_hal.cpp"
cat > "$GPIO_HAL" << 'EOF'
/**
 * @file gpio_hal.cpp
 * @brief GPIO Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

static bool gpio_initialized = false;

hal_status_t hal_gpio_init(void) {
    if (gpio_initialized) {
        return HAL_OK;
    }
    
    printf("[GPIO] Initializing GPIO subsystem...\n");
    
    // Configure status LEDs as outputs
    gpio_init(LED_STATUS_PIN);
    gpio_set_dir(LED_STATUS_PIN, GPIO_OUT);
    gpio_put(LED_STATUS_PIN, 0);
    
    gpio_init(LED_ERROR_PIN);
    gpio_set_dir(LED_ERROR_PIN, GPIO_OUT);
    gpio_put(LED_ERROR_PIN, 0);
    
    gpio_init(LED_COMM_PIN);
    gpio_set_dir(LED_COMM_PIN, GPIO_OUT);
    gpio_put(LED_COMM_PIN, 0);
    
    gpio_init(LED_POWER_PIN);
    gpio_set_dir(LED_POWER_PIN, GPIO_OUT);
    gpio_put(LED_POWER_PIN, 1); // Power LED on
    
    // Configure buttons as inputs with pull-ups
    gpio_init(BTN_USER_PIN);
    gpio_set_dir(BTN_USER_PIN, GPIO_IN);
    gpio_pull_up(BTN_USER_PIN);
    
    gpio_init(BTN_RESET_PIN);
    gpio_set_dir(BTN_RESET_PIN, GPIO_IN);
    gpio_pull_up(BTN_RESET_PIN);
    
    gpio_init(BTN_MODE_PIN);
    gpio_set_dir(BTN_MODE_PIN, GPIO_IN);
    gpio_pull_up(BTN_MODE_PIN);
    
    gpio_initialized = true;
    printf("[GPIO] GPIO subsystem initialized\n");
    
    return HAL_OK;
}

hal_status_t hal_gpio_config(uint32_t pin, gpio_mode_t mode) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_init(pin);
    
    switch (mode) {
        case GPIO_INPUT:
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;
        case GPIO_OUTPUT:
            gpio_set_dir(pin, GPIO_OUT);
            break;
        case GPIO_INPUT_PULLUP:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;
        case GPIO_INPUT_PULLDOWN:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;
        case GPIO_OPEN_DRAIN:
            // Pico doesn't have true open drain, simulate it
            gpio_set_dir(pin, GPIO_OUT);
            break;
        default:
            return HAL_INVALID_PARAM;
    }
    
    return HAL_OK;
}

hal_status_t hal_gpio_write(uint32_t pin, gpio_state_t state) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_put(pin, (state == GPIO_HIGH) ? 1 : 0);
    return HAL_OK;
}

hal_status_t hal_gpio_read(uint32_t pin, gpio_state_t *state) {
    if (!gpio_initialized || !state) {
        return HAL_ERROR;
    }
    
    *state = gpio_get(pin) ? GPIO_HIGH : GPIO_LOW;
    return HAL_OK;
}

hal_status_t hal_gpio_toggle(uint32_t pin) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_put(pin, !gpio_get(pin));
    return HAL_OK;
}

hal_status_t hal_gpio_interrupt_enable(uint32_t pin, uint8_t trigger_edge, void (*callback)(uint32_t)) {
    if (!gpio_initialized || !callback) {
        return HAL_ERROR;
    }
    
    // Store callback (simplified - real implementation would need callback storage)
    uint32_t events = 0;
    if (trigger_edge & 1) events |= GPIO_IRQ_EDGE_RISE;
    if (trigger_edge & 2) events |= GPIO_IRQ_EDGE_FALL;
    
    gpio_set_irq_enabled_with_callback(pin, events, true, (gpio_irq_callback_t)callback);
    
    return HAL_OK;
}

hal_status_t hal_gpio_interrupt_disable(uint32_t pin) {
    if (!gpio_initialized) {
        return HAL_ERROR;
    }
    
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    return HAL_OK;
}
EOF
echo -e "${GREEN}✓ Created proper gpio_hal.cpp${NC}"

# 4. Fix display_hal.cpp format warnings by using proper format specifiers
echo -e "${YELLOW}4. Fixing format warnings in display_hal.cpp...${NC}"
DISPLAY_HAL="$PROJECT_ROOT/platforms/pico_w/hal/display_hal.cpp"
if [ -f "$DISPLAY_HAL" ]; then
    # Fix the format specifiers for uint32_t
    sed -i.bak 's/%06X/%06lX/g' "$DISPLAY_HAL"
    sed -i.bak 's/%04X/%04lX/g' "$DISPLAY_HAL"
    echo -e "${GREEN}✓ Fixed format warnings in display_hal.cpp${NC}"
fi

# 5. Create a proper UART HAL file (since gpio_hal.cpp incorrectly contained UART code)
echo -e "${YELLOW}5. Ensuring uart_hal.cpp is correct...${NC}"
UART_HAL="$PROJECT_ROOT/platforms/pico_w/hal/uart_hal.cpp"
if [ ! -f "$UART_HAL" ] || grep -q "uart_data_bits_t" "$UART_HAL"; then
    cat > "$UART_HAL" << 'EOF'
/**
 * @file uart_hal.cpp
 * @brief UART Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
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
EOF
    echo -e "${GREEN}✓ Created proper uart_hal.cpp${NC}"
fi

# 6. Create missing SPI and I2C HAL stubs
echo -e "${YELLOW}6. Creating missing HAL stubs...${NC}"

# SPI HAL stub
cat > "$PROJECT_ROOT/platforms/pico_w/hal/spi_hal.cpp" << 'EOF'
#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_spi_init(uint8_t spi_id, const spi_config_t *config) {
    printf("[SPI] SPI%d init (stub)\n", spi_id);
    return HAL_OK;
}

hal_status_t hal_spi_deinit(uint8_t spi_id) {
    return HAL_OK;
}

hal_status_t hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_data, uint8_t *rx_data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_spi_set_cs(uint8_t spi_id, uint32_t cs_pin, bool active) {
    return HAL_NOT_SUPPORTED;
}
EOF

# I2C HAL stub
cat > "$PROJECT_ROOT/platforms/pico_w/hal/i2c_hal.cpp" << 'EOF'
#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_i2c_init(uint8_t i2c_id, const i2c_config_t *config) {
    printf("[I2C] I2C%d init (stub)\n", i2c_id);
    return HAL_OK;
}

hal_status_t hal_i2c_deinit(uint8_t i2c_id) {
    return HAL_OK;
}

hal_status_t hal_i2c_transmit(uint8_t i2c_id, uint8_t device_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_receive(uint8_t i2c_id, uint8_t device_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_write_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_read_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}
EOF

# Timer HAL stub
cat > "$PROJECT_ROOT/platforms/pico_w/hal/timer_hal.cpp" << 'EOF'
#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_timer_init(uint8_t timer_id, const timer_config_t *config) {
    printf("[TIMER] Timer%d init (stub)\n", timer_id);
    return HAL_OK;
}

hal_status_t hal_timer_deinit(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_start(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_stop(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_get_count(uint8_t timer_id, uint32_t *count) {
    if (count) *count = 0;
    return HAL_OK;
}

hal_status_t hal_timer_reset(uint8_t timer_id) {
    return HAL_OK;
}
EOF

# PWM HAL stub
cat > "$PROJECT_ROOT/platforms/pico_w/hal/pwm_hal.cpp" << 'EOF'
#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_pwm_init(uint8_t pwm_id, uint32_t frequency_hz) {
    printf("[PWM] PWM%d init at %lu Hz (stub)\n", pwm_id, frequency_hz);
    return HAL_OK;
}

hal_status_t hal_pwm_deinit(uint8_t pwm_id) {
    return HAL_OK;
}

hal_status_t hal_pwm_set_duty(uint8_t pwm_id, uint8_t channel, float duty_percent) {
    return HAL_OK;
}

hal_status_t hal_pwm_start(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}

hal_status_t hal_pwm_stop(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}
EOF

echo -e "${GREEN}✓ Created HAL stubs${NC}"

# 7. Test the build
echo ""
echo -e "${BLUE}Testing the build after fixes...${NC}"

if [ -z "$PICO_SDK_PATH" ]; then
    echo -e "${RED}ERROR: PICO_SDK_PATH not set${NC}"
    echo "Please run: export PICO_SDK_PATH=/path/to/pico-sdk"
    exit 1
fi

BUILD_DIR="$PROJECT_ROOT/build/pico_w_fixed"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${YELLOW}Running CMake configure...${NC}"

if cmake -DCMAKE_BUILD_TYPE=Release \
         -DPICO_BOARD=pico_w \
         -DPICO_PLATFORM=rp2040 \
         ../../platforms/pico_w; then
    echo -e "${GREEN}✓ CMake configure successful${NC}"
    
    echo -e "${YELLOW}Building...${NC}"
    if make -j4; then
        echo -e "${GREEN}✅ BUILD SUCCESSFUL!${NC}"
        echo ""
        echo -e "${BLUE}Build outputs:${NC}"
        ls -la diagnostic_rig_pico.*
        echo ""
        echo -e "${GREEN}Your UF2 file is ready: $(pwd)/diagnostic_rig_pico.uf2${NC}"
    else
        echo -e "${RED}✗ Build still has errors${NC}"
        echo "Check the output above for remaining issues"
        exit 1
    fi
else
    echo -e "${RED}✗ CMake configure failed${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}=== All Build Errors Fixed! ===${NC}"
echo "You can now use your existing scripts:"
echo "  ./flash.sh pico"
echo "  ./monitor.sh pico"