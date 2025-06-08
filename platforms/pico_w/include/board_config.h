/**
 * @file board_config.h
 * @brief Board configuration for Raspberry Pi Pico W with WiFi support
 *
 * This file contains all pin mappings, hardware configurations, and
 * platform-specific constants for the Pico W board including WiFi functionality.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef PICO_W_BOARD_CONFIG_H
#define PICO_W_BOARD_CONFIG_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

// Standard Pico SDK includes
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

// WiFi includes are handled in source files, not here

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // BOARD IDENTIFICATION
    // =============================================================================

#define BOARD_NAME "Raspberry Pi Pico W"
#define BOARD_VERSION "1.0"
#define MCU_NAME "RP2040"
#define BOARD_HAS_WIFI 1
#define BOARD_HAS_BLUETOOTH 0
#define BOARD_HAS_CAN 0
#define BOARD_HAS_DAC 0


    // =============================================================================
    // SYSTEM CONFIGURATION
    // =============================================================================

#define SYSTEM_TICK_FREQ_HZ 1000 // 1ms tick
#define WATCHDOG_TIMEOUT_MS 8000 // 8 second watchdog
#define STACK_SIZE_BYTES 4096    // Main stack size
#define HEAP_SIZE_BYTES 32768    // Heap size

// =============================================================================
// GPIO PIN DEFINITIONS
// =============================================================================

// Status LEDs
#define LED_STATUS_PIN PICO_DEFAULT_LED_PIN // GPIO 25 (built-in LED - note: shares with WiFi on Pico W)
#define LED_ERROR_PIN 16                    // External error LED
#define LED_COMM_PIN 17                     // Communication activity LED
#define LED_POWER_PIN 18                    // Power status LED

// User Inputs
#define BTN_USER_PIN 14  // User button
#define BTN_RESET_PIN 15 // Reset button
#define BTN_MODE_PIN 19  // Mode selection button

// Diagnostic Channel GPIOs
#define DIAG_CH1_ENABLE_PIN 20 // Channel 1 enable
#define DIAG_CH2_ENABLE_PIN 21 // Channel 2 enable
#define DIAG_CH3_ENABLE_PIN 22 // Channel 3 enable
#define DIAG_CH4_ENABLE_PIN 26 // Channel 4 enable

// Control Outputs
#define RELAY_1_PIN 6     // Relay control 1
#define RELAY_2_PIN 7     // Relay control 2
#define BUZZER_PIN 8      // Buzzer output
#define FAN_CONTROL_PIN 9 // Fan speed control (PWM)

// External Interface
#define EXT_INT_PIN 10    // External interrupt input
#define ENABLE_3V3_PIN 11 // 3.3V rail enable
#define ENABLE_5V_PIN 12  // 5V rail enable

    // =============================================================================
    // ADC CONFIGURATION
    // =============================================================================

#define ADC_NUM_CHANNELS 4
#define ADC_RESOLUTION_BITS 12
#define ADC_REFERENCE_VOLTAGE 3.3f
#define ADC_SAMPLE_TIME_US 10

// ADC Channel Assignments
#define ADC_CH1_VOLTAGE 0 // GPIO 26 - Channel 1 voltage sense
#define ADC_CH2_VOLTAGE 1 // GPIO 27 - Channel 2 voltage sense
#define ADC_CH3_CURRENT 2 // GPIO 28 - Channel 3 current sense
#define ADC_TEMPERATURE 3 // ADC 4 - Internal temperature

// ADC Pin mappings (GPIO numbers)
#define ADC_CH1_VOLTAGE_PIN 26
#define ADC_CH2_VOLTAGE_PIN 27
#define ADC_CH3_CURRENT_PIN 28
// Note: Temperature sensor is internal, no GPIO pin

// ADC Scaling factors
#define ADC_VOLTAGE_SCALE 0.01  // V/count scaling
#define ADC_CURRENT_SCALE 0.001 // A/count scaling

    // =============================================================================
    // UART CONFIGURATION
    // =============================================================================

#define UART_DEBUG_ID uart0
#define UART_DEBUG_TX_PIN 0
#define UART_DEBUG_RX_PIN 1
#define UART_DEBUG_BAUDRATE 115200

#define UART_EXT_ID uart1
#define UART_EXT_TX_PIN 4
#define UART_EXT_RX_PIN 5
#define UART_EXT_BAUDRATE 9600

// UART Buffer sizes
#define UART_TX_BUFFER_SIZE 512
#define UART_RX_BUFFER_SIZE 512

    // =============================================================================
    // SPI CONFIGURATION
    // =============================================================================

#define SPI_DISPLAY_ID spi0
#define SPI_DISPLAY_SCK_PIN 2
#define SPI_DISPLAY_MOSI_PIN 3
#define SPI_DISPLAY_MISO_PIN 4 // Not used for display
#define SPI_DISPLAY_CS_PIN 5
#define SPI_DISPLAY_DC_PIN 6           // Data/Command pin
#define SPI_DISPLAY_RST_PIN 7          // Reset pin
#define SPI_DISPLAY_FREQUENCY 10000000 // 10 MHz

#define SPI_EXT_ID spi1
#define SPI_EXT_SCK_PIN 10
#define SPI_EXT_MOSI_PIN 11
#define SPI_EXT_MISO_PIN 12
#define SPI_EXT_CS_PIN 13
#define SPI_EXT_FREQUENCY 1000000 // 1 MHz

    // =============================================================================
    // I2C CONFIGURATION
    // =============================================================================

#define I2C_SENSORS_ID i2c0
#define I2C_SENSORS_SDA_PIN 8
#define I2C_SENSORS_SCL_PIN 9
#define I2C_SENSORS_FREQUENCY 100000 // 100 kHz standard mode

#define I2C_EXT_ID i2c1
#define I2C_EXT_SDA_PIN 14
#define I2C_EXT_SCL_PIN 15
#define I2C_EXT_FREQUENCY 400000 // 400 kHz fast mode

// Common I2C device addresses
#define I2C_ADDR_TEMP_SENSOR 0x48 // Temperature sensor
#define I2C_ADDR_EEPROM 0x50      // Configuration EEPROM
#define I2C_ADDR_RTC 0x68         // Real-time clock

    // =============================================================================
    // PWM CONFIGURATION
    // =============================================================================

#define PWM_FAN_SLICE 4            // PWM slice for fan control
#define PWM_FAN_CHANNEL PWM_CHAN_B // PWM channel for fan
#define PWM_FAN_FREQUENCY 25000    // 25 kHz PWM frequency

#define PWM_BUZZER_SLICE 5            // PWM slice for buzzer
#define PWM_BUZZER_CHANNEL PWM_CHAN_A // PWM channel for buzzer
#define PWM_BUZZER_FREQUENCY 2000     // 2 kHz buzzer frequency

    // =============================================================================
    // WIFI CONFIGURATION (Enhanced for Pico W)
    // =============================================================================

// WiFi Enable and Mode
#define WIFI_ENABLED 1
#define WIFI_USE_CYW43 1
#define WIFI_MODE_STA 1 // Station mode (client)
#define WIFI_MODE_AP 0  // Access Point mode (disabled by default)

// WiFi Credentials and Network Settings
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define WIFI_HOSTNAME "pico-diagnostic-rig"
#define WIFI_CONNECT_TIMEOUT_MS 30000      // 30 seconds
#define WIFI_RECONNECT_DELAY_MS 5000       // 5 seconds
#define WIFI_MAX_RETRY_COUNT 5            // Maximum connection retries
#define WIFI_STATUS_CHECK_INTERVAL_MS 1000 // Check connection status every 1s

// Default WiFi credentials (change these for your network)
#define WIFI_DEFAULT_SSID "YourWiFiNetwork"
#define WIFI_DEFAULT_PASSWORD "YourWiFiPassword"
#define USE_HARDCODED_WIFI false // Set to true to use hardcoded credentials

// WiFi Power Management
#define WIFI_PM_NONE 0        // No power management
#define WIFI_PM_PERFORMANCE 1 // Performance mode
#define WIFI_PM_POWERSAVE 2   // Power save mode
#define WIFI_POWER_MODE WIFI_PM_PERFORMANCE

// WiFi LED Status (shares with built-in LED on Pico W)
#define WIFI_LED_ENABLED 1
#define WIFI_LED_BLINK_CONNECTED_MS 1000 // Slow blink when connected
#define WIFI_LED_BLINK_CONNECTING_MS 200 // Fast blink when connecting
#define WIFI_LED_SOLID_DISCONNECTED 0    // Off when disconnected

// Network settings
#define NET_HTTP_PORT 80
#define NET_WEBSOCKET_PORT 8080
#define NET_TELNET_PORT 23
#define NET_TCP_PORT 8081
#define NET_UDP_PORT 8082
#define NET_MAX_CONNECTIONS 4
#define NET_BUFFER_SIZE 1024
#define NET_TIMEOUT_MS 10000

// DHCP Settings
#define DHCP_ENABLED 1
#define DHCP_TIMEOUT_MS 30000

// Static IP Settings (used when DHCP disabled)
#define STATIC_IP_ENABLED 0
#define STATIC_IP_ADDR "192.168.1.100"
#define STATIC_IP_NETMASK "255.255.255.0"
#define STATIC_IP_GATEWAY "192.168.1.1"
#define STATIC_IP_DNS "8.8.8.8"

// WiFi Security (removed - defined as enums in wifi_manager.h)
#define WIFI_DEFAULT_AUTH_MODE 3 // WPA2_PSK equivalent

    // =============================================================================
    // WEBSOCKET CONFIGURATION
    // =============================================================================

#define WEBSOCKET_ENABLED 1
#define WEBSOCKET_MAX_CLIENTS 4
#define WEBSOCKET_BUFFER_SIZE 1024
#define WEBSOCKET_PING_INTERVAL_MS 30000 // 30 seconds
#define WEBSOCKET_TIMEOUT_MS 60000       // 1 minute timeout
#define WEBSOCKET_MAX_FRAME_SIZE 4096

// WebSocket message types
#define WS_MSG_TEXT 0x01
#define WS_MSG_BINARY 0x02
#define WS_MSG_CLOSE 0x08
#define WS_MSG_PING 0x09
#define WS_MSG_PONG 0x0A

    // =============================================================================
    // HTTP SERVER CONFIGURATION
    // =============================================================================

#define HTTP_SERVER_ENABLED 1
#define HTTP_MAX_CONCURRENT_CONNECTIONS 4
#define HTTP_REQUEST_BUFFER_SIZE 2048
#define HTTP_RESPONSE_BUFFER_SIZE 4096
#define HTTP_TIMEOUT_MS 30000
#define HTTP_KEEPALIVE_TIMEOUT_MS 5000

// HTTP Response codes
#define HTTP_200_OK "200 OK"
#define HTTP_404_NOT_FOUND "404 Not Found"
#define HTTP_500_INTERNAL_ERROR "500 Internal Server Error"

    // =============================================================================
    // DISPLAY CONFIGURATION (WEB-BASED FOR PICO W)
    // =============================================================================

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define DISPLAY_COLOR_DEPTH 16     // 16-bit color
#define DISPLAY_UPDATE_RATE_MS 100 // Update every 100ms
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)

// Web display simulation
#define WEB_DISPLAY_ENABLED 1
#define WEB_DISPLAY_WEBSOCKET 1
#define WEB_DISPLAY_CANVAS 1
#define WEB_DISPLAY_FRAMERATE 10 // 10 FPS

    // =============================================================================
    // TIMING CONFIGURATION
    // =============================================================================

#define TIMER_SYSTEM_ID 0     // System timer
#define TIMER_DIAGNOSTIC_ID 1 // Diagnostic timer
#define TIMER_HEARTBEAT_ID 2  // Heartbeat timer
#define TIMER_WIFI_ID 3       // WiFi status timer

#define HEARTBEAT_INTERVAL_MS 1000   // 1 second heartbeat
#define DIAGNOSTIC_INTERVAL_MS 50    // 50ms diagnostic cycle
#define SYSTEM_TICK_INTERVAL_MS 1    // 1ms system tick
#define WIFI_STATUS_INTERVAL_MS 5000 // 5 second WiFi status check

    // =============================================================================
    // INTERRUPT PRIORITIES
    // =============================================================================

#define IRQ_PRIORITY_HIGHEST 0
#define IRQ_PRIORITY_HIGH 1
#define IRQ_PRIORITY_MEDIUM 2
#define IRQ_PRIORITY_LOW 3

// Specific interrupt priorities
#define IRQ_PRIORITY_SYSTEM IRQ_PRIORITY_HIGHEST
#define IRQ_PRIORITY_WIFI IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_UART IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_SPI IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_I2C IRQ_PRIORITY_MEDIUM
#define IRQ_PRIORITY_GPIO IRQ_PRIORITY_MEDIUM
#define IRQ_PRIORITY_ADC IRQ_PRIORITY_LOW
#define IRQ_PRIORITY_TIMER IRQ_PRIORITY_LOW

    // =============================================================================
    // MEMORY CONFIGURATION
    // =============================================================================

#define FLASH_SIZE_BYTES (2 * 1024 * 1024) // 2MB Flash
#define RAM_SIZE_BYTES (264 * 1024)        // 264KB RAM
#define CONFIG_FLASH_OFFSET (1024 * 1024)  // 1MB offset for config
#define CONFIG_FLASH_SIZE (64 * 1024)      // 64KB for config

// Buffer sizes
#define LOG_BUFFER_SIZE 2048
#define DATA_BUFFER_SIZE 1024
#define WEB_BUFFER_SIZE 4096
#define WIFI_BUFFER_SIZE 2048

    // =============================================================================
    // DIAGNOSTIC CHANNEL CONFIGURATION
    // =============================================================================

#define NUM_DIAGNOSTIC_CHANNELS 4
#define CHANNEL_VOLTAGE_RANGE 24.0f // Maximum voltage per channel
#define CHANNEL_CURRENT_RANGE 5.0f  // Maximum current per channel
#define CHANNEL_SAMPLE_RATE_HZ 1000 // Sample rate per channel

// Channel configuration structure indices
#define CHANNEL_1_CONFIG_IDX 0
#define CHANNEL_2_CONFIG_IDX 1
#define CHANNEL_3_CONFIG_IDX 2
#define CHANNEL_4_CONFIG_IDX 3

    // =============================================================================
    // SAFETY LIMITS
    // =============================================================================

#define SAFETY_VOLTAGE_MAX 30.0f // Maximum safe voltage
#define SAFETY_CURRENT_MAX 10.0f // Maximum safe current
#define SAFETY_TEMP_MAX 85.0f    // Maximum operating temperature
#define SAFETY_TEMP_MIN -10.0f   // Minimum operating temperature

// Emergency shutdown thresholds
#define EMERGENCY_VOLTAGE_LIMIT 35.0f
#define EMERGENCY_CURRENT_LIMIT 12.0f
#define EMERGENCY_TEMP_LIMIT 95.0f

    // =============================================================================
    // CALIBRATION CONSTANTS
    // =============================================================================

#define CAL_VOLTAGE_OFFSET 0.0f // Voltage measurement offset
#define CAL_VOLTAGE_GAIN 1.0f   // Voltage measurement gain
#define CAL_CURRENT_OFFSET 0.0f // Current measurement offset
#define CAL_CURRENT_GAIN 1.0f   // Current measurement gain
#define CAL_TEMP_OFFSET 0.0f    // Temperature measurement offset
#define CAL_TEMP_GAIN 1.0f      // Temperature measurement gain

    // =============================================================================
    // DEBUG AND LOGGING
    // =============================================================================

#define DEBUG_ENABLED 1
#define DEBUG_LEVEL_ERROR 0
#define DEBUG_LEVEL_WARN 1
#define DEBUG_LEVEL_INFO 2
#define DEBUG_LEVEL_DEBUG 3
#define DEBUG_LEVEL_VERBOSE 4

#define DEFAULT_DEBUG_LEVEL DEBUG_LEVEL_INFO

// Log destinations
#define LOG_TO_UART 1
#define LOG_TO_WEB 1
#define LOG_TO_WEBSOCKET 1
#define LOG_TO_FLASH 0 // Disabled by default

    // =============================================================================
    // WIFI STATUS DEFINITIONS
    // =============================================================================

// WiFi connection states (for internal use - don't conflict with enums)
#define WIFI_STATE_DISCONNECTED 0
#define WIFI_STATE_CONNECTING 1
#define WIFI_STATE_CONNECTED 2
#define WIFI_STATE_FAILED 3
#define WIFI_STATE_TIMEOUT 4
#define WIFI_STATE_AP_MODE 5

    // Remove conflicting event and auth definitions - these will be in wifi_manager.h as enums

    // =============================================================================
    // HELPER MACROS
    // =============================================================================

// GPIO helper macros
#define GPIO_SET_HIGH(pin) gpio_put(pin, 1)
#define GPIO_SET_LOW(pin) gpio_put(pin, 0)
#define GPIO_READ(pin) gpio_get(pin)
#define GPIO_TOGGLE(pin) gpio_put(pin, !gpio_get(pin))

// ADC helper macros
#define ADC_TO_VOLTAGE(val) ((val) * ADC_REFERENCE_VOLTAGE / (1 << ADC_RESOLUTION_BITS))
#define VOLTAGE_TO_ADC(volt) ((uint16_t)((volt) * (1 << ADC_RESOLUTION_BITS) / ADC_REFERENCE_VOLTAGE))

// PWM helper macros
#define PWM_PERCENT_TO_LEVEL(percent, wrap) ((wrap) * (percent) / 100)
#define PWM_LEVEL_TO_PERCENT(level, wrap) ((level) * 100 / (wrap))

// Timing helper macros
#define MS_TO_US(ms) ((ms) * 1000)
#define US_TO_MS(us) ((us) / 1000)
#define SEC_TO_MS(sec) ((sec) * 1000)
#define MIN_TO_MS(min) ((min) * 60 * 1000)

// WiFi helper macros (implementation in wifi_manager.cpp)
#define WIFI_IS_CONNECTED() wifi_is_connected()
#define WIFI_SET_LED(state) wifi_set_led(state)
#define WIFI_TOGGLE_LED() wifi_toggle_led()

    // =============================================================================
    // COMPATIBILITY CONSTANTS
    // =============================================================================

#ifndef MAIN_LOOP_DELAY_MS
#define MAIN_LOOP_DELAY_MS 100
#endif

#ifndef STATUS_UPDATE_INTERVAL_MS
#define STATUS_UPDATE_INTERVAL_MS 5000
#endif

#ifndef SAFETY_CHECK_INTERVAL_MS
#define SAFETY_CHECK_INTERVAL_MS 500
#endif

    // Remove duplicate wifi_config_t and network_status_t - these will be in wifi_manager.h

#ifdef __cplusplus
}
#endif

#endif // PICO_W_BOARD_CONFIG_H
// =============================================================================
// GPIO STATE CONSTANTS (Added during consolidation)
// =============================================================================

#ifndef GPIO_STATE_HIGH
#define GPIO_STATE_HIGH     1
#endif

#ifndef GPIO_STATE_LOW
#define GPIO_STATE_LOW      0
#endif

