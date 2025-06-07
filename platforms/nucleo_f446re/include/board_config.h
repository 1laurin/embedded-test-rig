/**
 * @file board_config.h
 * @brief Board configuration for STM32 Nucleo-F446RE
 *
 * This file contains all pin mappings, hardware configurations, and
 * platform-specific constants for the STM32 Nucleo-F446RE board.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef STM32_NUCLEO_F446RE_BOARD_CONFIG_H
#define STM32_NUCLEO_F446RE_BOARD_CONFIG_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // BOARD IDENTIFICATION
    // =============================================================================

#define BOARD_NAME "STM32 Nucleo-F446RE"
#define BOARD_VERSION "1.0"
#define MCU_NAME "STM32F446RE"
#define MCU_FREQUENCY_HZ 180000000UL // 180 MHz maximum
#define BOARD_HAS_WIFI 0
#define BOARD_HAS_BLUETOOTH 0
#define BOARD_HAS_CAN 1
#define BOARD_HAS_DAC 1

    // =============================================================================
    // SYSTEM CONFIGURATION
    // =============================================================================

#define SYSTEM_TICK_FREQ_HZ 1000  // 1ms tick
#define WATCHDOG_TIMEOUT_MS 10000 // 10 second watchdog
#define STACK_SIZE_BYTES 8192     // Main stack size
#define HEAP_SIZE_BYTES 65536     // Heap size

// Clock configuration
#define HSE_VALUE 8000000UL  // 8 MHz external crystal
#define HSI_VALUE 16000000UL // 16 MHz internal RC
#define LSE_VALUE 32768UL    // 32.768 kHz external crystal
#define LSI_VALUE 32000UL    // ~32 kHz internal RC

// =============================================================================
// GPIO PIN DEFINITIONS
// =============================================================================

// User LED (built-in green LED)
#define LED_STATUS_PORT GPIOA
#define LED_STATUS_PIN GPIO_PIN_5

// Additional Status LEDs (external)
#define LED_ERROR_PORT GPIOB
#define LED_ERROR_PIN GPIO_PIN_0
#define LED_COMM_PORT GPIOB
#define LED_COMM_PIN GPIO_PIN_1
#define LED_POWER_PORT GPIOB
#define LED_POWER_PIN GPIO_PIN_2

// User Button (built-in blue button)
#define BTN_USER_PORT GPIOC
#define BTN_USER_PIN GPIO_PIN_13

// Additional Buttons (external)
#define BTN_RESET_PORT GPIOC
#define BTN_RESET_PIN GPIO_PIN_14
#define BTN_MODE_PORT GPIOC
#define BTN_MODE_PIN GPIO_PIN_15

// Diagnostic Channel Control
#define DIAG_CH1_ENABLE_PORT GPIOA
#define DIAG_CH1_ENABLE_PIN GPIO_PIN_8
#define DIAG_CH2_ENABLE_PORT GPIOA
#define DIAG_CH2_ENABLE_PIN GPIO_PIN_9
#define DIAG_CH3_ENABLE_PORT GPIOA
#define DIAG_CH3_ENABLE_PIN GPIO_PIN_10
#define DIAG_CH4_ENABLE_PORT GPIOA
#define DIAG_CH4_ENABLE_PIN GPIO_PIN_11

// Relay Controls
#define RELAY_1_PORT GPIOB
#define RELAY_1_PIN GPIO_PIN_12
#define RELAY_2_PORT GPIOB
#define RELAY_2_PIN GPIO_PIN_13

// Buzzer Control
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_PIN_14

// Fan Control (PWM)
#define FAN_CONTROL_PORT GPIOB
#define FAN_CONTROL_PIN GPIO_PIN_15

// Power Enable Controls
#define ENABLE_3V3_PORT GPIOC
#define ENABLE_3V3_PIN GPIO_PIN_0
#define ENABLE_5V_PORT GPIOC
#define ENABLE_5V_PIN GPIO_PIN_1

// External Interrupt
#define EXT_INT_PORT GPIOC
#define EXT_INT_PIN GPIO_PIN_2

    // =============================================================================
    // ADC CONFIGURATION
    // =============================================================================

#define ADC_NUM_CHANNELS 8
#define ADC_RESOLUTION_BITS 12
#define ADC_REFERENCE_VOLTAGE 3.3f
#define ADC_SAMPLE_TIME ADC_SAMPLETIME_15CYCLES

// ADC Instance
#define ADC_INSTANCE ADC1
#define ADC_CLOCK_ENABLE() __HAL_RCC_ADC1_CLK_ENABLE()

// ADC Channel Assignments
#define ADC_CH1_VOLTAGE ADC_CHANNEL_0 // PA0 - Channel 1 voltage
#define ADC_CH2_VOLTAGE ADC_CHANNEL_1 // PA1 - Channel 2 voltage
#define ADC_CH3_VOLTAGE ADC_CHANNEL_4 // PA4 - Channel 3 voltage
#define ADC_CH4_VOLTAGE ADC_CHANNEL_5 // PA5 - Channel 4 voltage
#define ADC_CH1_CURRENT ADC_CHANNEL_6 // PA6 - Channel 1 current
#define ADC_CH2_CURRENT ADC_CHANNEL_7 // PA7 - Channel 2 current
#define ADC_CH3_CURRENT ADC_CHANNEL_8 // PB0 - Channel 3 current
#define ADC_CH4_CURRENT ADC_CHANNEL_9 // PB1 - Channel 4 current

// ADC GPIO Configuration
#define ADC_CH1_VOLTAGE_PORT GPIOA
#define ADC_CH1_VOLTAGE_PIN GPIO_PIN_0
#define ADC_CH2_VOLTAGE_PORT GPIOA
#define ADC_CH2_VOLTAGE_PIN GPIO_PIN_1
#define ADC_CH3_VOLTAGE_PORT GPIOA
#define ADC_CH3_VOLTAGE_PIN GPIO_PIN_4
#define ADC_CH4_VOLTAGE_PORT GPIOA
#define ADC_CH4_VOLTAGE_PIN GPIO_PIN_5

#define ADC_CH1_CURRENT_PORT GPIOA
#define ADC_CH1_CURRENT_PIN GPIO_PIN_6
#define ADC_CH2_CURRENT_PORT GPIOA
#define ADC_CH2_CURRENT_PIN GPIO_PIN_7
#define ADC_CH3_CURRENT_PORT GPIOB
#define ADC_CH3_CURRENT_PIN GPIO_PIN_0
#define ADC_CH4_CURRENT_PORT GPIOB
#define ADC_CH4_CURRENT_PIN GPIO_PIN_1

// ADC Scaling factors
#define ADC_VOLTAGE_SCALE 0.01  // V/count scaling
#define ADC_CURRENT_SCALE 0.001 // A/count scaling

    // =============================================================================
    // DAC CONFIGURATION
    // =============================================================================

#define DAC_INSTANCE DAC
#define DAC_CLOCK_ENABLE() __HAL_RCC_DAC_CLK_ENABLE()

// DAC Channels
#define DAC_CH1_PORT GPIOA
#define DAC_CH1_PIN GPIO_PIN_4
#define DAC_CH1_CHANNEL DAC_CHANNEL_1

#define DAC_CH2_PORT GPIOA
#define DAC_CH2_PIN GPIO_PIN_5
#define DAC_CH2_CHANNEL DAC_CHANNEL_2

#define DAC_RESOLUTION_BITS 12
#define DAC_REFERENCE_VOLTAGE 3.3f

// =============================================================================
// UART CONFIGURATION
// =============================================================================

// Debug UART (ST-Link Virtual COM Port)
#define UART_DEBUG_INSTANCE USART2
#define UART_DEBUG_CLOCK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()
#define UART_DEBUG_TX_PORT GPIOA
#define UART_DEBUG_TX_PIN GPIO_PIN_2
#define UART_DEBUG_RX_PORT GPIOA
#define UART_DEBUG_RX_PIN GPIO_PIN_3
#define UART_DEBUG_AF GPIO_AF7_USART2
#define UART_DEBUG_BAUDRATE 115200
#define UART_DEBUG_IRQn USART2_IRQn

// External UART
#define UART_EXT_INSTANCE USART1
#define UART_EXT_CLOCK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#define UART_EXT_TX_PORT GPIOA
#define UART_EXT_TX_PIN GPIO_PIN_9
#define UART_EXT_RX_PORT GPIOA
#define UART_EXT_RX_PIN GPIO_PIN_10
#define UART_EXT_AF GPIO_AF7_USART1
#define UART_EXT_BAUDRATE 9600
#define UART_EXT_IRQn USART1_IRQn

// UART Buffer sizes
#define UART_TX_BUFFER_SIZE 1024
#define UART_RX_BUFFER_SIZE 1024

// =============================================================================
// SPI CONFIGURATION
// =============================================================================

// Display SPI
#define SPI_DISPLAY_INSTANCE SPI1
#define SPI_DISPLAY_CLOCK_ENABLE() __HAL_RCC_SPI1_CLK_ENABLE()
#define SPI_DISPLAY_SCK_PORT GPIOA
#define SPI_DISPLAY_SCK_PIN GPIO_PIN_5
#define SPI_DISPLAY_MISO_PORT GPIOA
#define SPI_DISPLAY_MISO_PIN GPIO_PIN_6
#define SPI_DISPLAY_MOSI_PORT GPIOA
#define SPI_DISPLAY_MOSI_PIN GPIO_PIN_7
#define SPI_DISPLAY_AF GPIO_AF5_SPI1

// Display control pins
#define SPI_DISPLAY_CS_PORT GPIOB
#define SPI_DISPLAY_CS_PIN GPIO_PIN_6
#define SPI_DISPLAY_DC_PORT GPIOB
#define SPI_DISPLAY_DC_PIN GPIO_PIN_7
#define SPI_DISPLAY_RST_PORT GPIOB
#define SPI_DISPLAY_RST_PIN GPIO_PIN_8

// External SPI
#define SPI_EXT_INSTANCE SPI2
#define SPI_EXT_CLOCK_ENABLE() __HAL_RCC_SPI2_CLK_ENABLE()
#define SPI_EXT_SCK_PORT GPIOB
#define SPI_EXT_SCK_PIN GPIO_PIN_10
#define SPI_EXT_MISO_PORT GPIOB
#define SPI_EXT_MISO_PIN GPIO_PIN_14
#define SPI_EXT_MOSI_PORT GPIOB
#define SPI_EXT_MOSI_PIN GPIO_PIN_15
#define SPI_EXT_AF GPIO_AF5_SPI2
#define SPI_EXT_CS_PORT GPIOB
#define SPI_EXT_CS_PIN GPIO_PIN_12

// =============================================================================
// I2C CONFIGURATION
// =============================================================================

// Sensor I2C
#define I2C_SENSORS_INSTANCE I2C1
#define I2C_SENSORS_CLOCK_ENABLE() __HAL_RCC_I2C1_CLK_ENABLE()
#define I2C_SENSORS_SCL_PORT GPIOB
#define I2C_SENSORS_SCL_PIN GPIO_PIN_8
#define I2C_SENSORS_SDA_PORT GPIOB
#define I2C_SENSORS_SDA_PIN GPIO_PIN_9
#define I2C_SENSORS_AF GPIO_AF4_I2C1
#define I2C_SENSORS_FREQUENCY 100000 // 100 kHz

// External I2C
#define I2C_EXT_INSTANCE I2C2
#define I2C_EXT_CLOCK_ENABLE() __HAL_RCC_I2C2_CLK_ENABLE()
#define I2C_EXT_SCL_PORT GPIOB
#define I2C_EXT_SCL_PIN GPIO_PIN_10
#define I2C_EXT_SDA_PORT GPIOB
#define I2C_EXT_SDA_PIN GPIO_PIN_11
#define I2C_EXT_AF GPIO_AF4_I2C2
#define I2C_EXT_FREQUENCY 400000 // 400 kHz

// Common I2C device addresses
#define I2C_ADDR_TEMP_SENSOR 0x48 // Temperature sensor
#define I2C_ADDR_EEPROM 0x50      // Configuration EEPROM
#define I2C_ADDR_RTC 0x68         // Real-time clock

    // =============================================================================
    // CAN CONFIGURATION
    // =============================================================================

#define CAN_INSTANCE CAN1
#define CAN_CLOCK_ENABLE() __HAL_RCC_CAN1_CLK_ENABLE()
#define CAN_TX_PORT GPIOD
#define CAN_TX_PIN GPIO_PIN_1
#define CAN_RX_PORT GPIOD
#define CAN_RX_PIN GPIO_PIN_0
#define CAN_AF GPIO_AF9_CAN1
#define CAN_IRQn CAN1_RX0_IRQn

// CAN Timing (for 500 kbps @ 180 MHz APB1)
#define CAN_PRESCALER 18
#define CAN_SJW CAN_SJW_1TQ
#define CAN_BS1 CAN_BS1_13TQ
#define CAN_BS2 CAN_BS2_2TQ

// =============================================================================
// PWM/TIMER CONFIGURATION
// =============================================================================

// Fan Control PWM (TIM3)
#define PWM_FAN_TIMER TIM3
#define PWM_FAN_CLOCK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()
#define PWM_FAN_CHANNEL TIM_CHANNEL_4
#define PWM_FAN_PORT GPIOB
#define PWM_FAN_PIN GPIO_PIN_1
#define PWM_FAN_AF GPIO_AF2_TIM3
#define PWM_FAN_FREQUENCY 25000 // 25 kHz

// Buzzer PWM (TIM4)
#define PWM_BUZZER_TIMER TIM4
#define PWM_BUZZER_CLOCK_ENABLE() __HAL_RCC_TIM4_CLK_ENABLE()
#define PWM_BUZZER_CHANNEL TIM_CHANNEL_1
#define PWM_BUZZER_PORT GPIOB
#define PWM_BUZZER_PIN GPIO_PIN_6
#define PWM_BUZZER_AF GPIO_AF2_TIM4
#define PWM_BUZZER_FREQUENCY 2000 // 2 kHz

// System Timer (TIM2)
#define TIMER_SYSTEM TIM2
#define TIMER_SYSTEM_CLOCK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
#define TIMER_SYSTEM_IRQn TIM2_IRQn

// Diagnostic Timer (TIM5)
#define TIMER_DIAGNOSTIC TIM5
#define TIMER_DIAGNOSTIC_CLOCK_ENABLE() __HAL_RCC_TIM5_CLK_ENABLE()
#define TIMER_DIAGNOSTIC_IRQn TIM5_IRQn

    // =============================================================================
    // DISPLAY CONFIGURATION (ILI9481)
    // =============================================================================

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320
#define DISPLAY_COLOR_DEPTH 16 // 16-bit color (RGB565)
#define DISPLAY_ORIENTATION 0  // 0=Portrait, 1=Landscape
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)

// Display timing
#define DISPLAY_RESET_DELAY_MS 100
#define DISPLAY_INIT_DELAY_MS 150
#define DISPLAY_CMD_DELAY_US 10

    // =============================================================================
    // TIMING CONFIGURATION
    // =============================================================================

#define HEARTBEAT_INTERVAL_MS 1000 // 1 second heartbeat
#define DIAGNOSTIC_INTERVAL_MS 50  // 50ms diagnostic cycle
#define SYSTEM_TICK_INTERVAL_MS 1  // 1ms system tick

    // =============================================================================
    // INTERRUPT PRIORITIES
    // =============================================================================

#define IRQ_PRIORITY_HIGHEST 0
#define IRQ_PRIORITY_HIGH 1
#define IRQ_PRIORITY_MEDIUM 2
#define IRQ_PRIORITY_LOW 3

// Specific interrupt priorities
#define IRQ_PRIORITY_SYSTEM IRQ_PRIORITY_HIGHEST
#define IRQ_PRIORITY_CAN IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_UART IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_SPI IRQ_PRIORITY_HIGH
#define IRQ_PRIORITY_I2C IRQ_PRIORITY_MEDIUM
#define IRQ_PRIORITY_GPIO IRQ_PRIORITY_MEDIUM
#define IRQ_PRIORITY_ADC IRQ_PRIORITY_LOW
#define IRQ_PRIORITY_TIMER IRQ_PRIORITY_LOW

    // =============================================================================
    // MEMORY CONFIGURATION
    // =============================================================================

#define FLASH_SIZE_BYTES (512 * 1024)      // 512KB Flash
#define RAM_SIZE_BYTES (128 * 1024)        // 128KB RAM
#define CONFIG_FLASH_SECTOR FLASH_SECTOR_7 // Last sector for config
#define CONFIG_FLASH_SIZE (128 * 1024)     // 128KB for config

// Buffer sizes
#define LOG_BUFFER_SIZE 4096
#define DATA_BUFFER_SIZE 2048
#define CAN_TX_BUFFER_SIZE 256
#define CAN_RX_BUFFER_SIZE 256

    // =============================================================================
    // DIAGNOSTIC CHANNEL CONFIGURATION
    // =============================================================================

#define NUM_DIAGNOSTIC_CHANNELS 4
#define CHANNEL_VOLTAGE_RANGE 30.0f // Maximum voltage per channel
#define CHANNEL_CURRENT_RANGE 10.0f // Maximum current per channel
#define CHANNEL_SAMPLE_RATE_HZ 1000 // Sample rate per channel

    // =============================================================================
    // SAFETY LIMITS
    // =============================================================================

#define SAFETY_VOLTAGE_MAX 35.0f // Maximum safe voltage
#define SAFETY_CURRENT_MAX 12.0f // Maximum safe current
#define SAFETY_TEMP_MAX 85.0f    // Maximum operating temperature
#define SAFETY_TEMP_MIN -10.0f   // Minimum operating temperature

// Emergency shutdown thresholds
#define EMERGENCY_VOLTAGE_LIMIT 40.0f
#define EMERGENCY_CURRENT_LIMIT 15.0f
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
#define LOG_TO_CAN 1
#define LOG_TO_FLASH 1

// =============================================================================
// HELPER MACROS
// =============================================================================

// GPIO helper macros
#define GPIO_SET_HIGH(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define GPIO_SET_LOW(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define GPIO_READ(port, pin) HAL_GPIO_ReadPin(port, pin)
#define GPIO_TOGGLE(port, pin) HAL_GPIO_TogglePin(port, pin)

// ADC helper macros
#define ADC_TO_VOLTAGE(val) ((val) * ADC_REFERENCE_VOLTAGE / (1 << ADC_RESOLUTION_BITS))
#define VOLTAGE_TO_ADC(volt) ((uint16_t)((volt) * (1 << ADC_RESOLUTION_BITS) / ADC_REFERENCE_VOLTAGE))

// DAC helper macros
#define DAC_TO_VOLTAGE(val) ((val) * DAC_REFERENCE_VOLTAGE / (1 << DAC_RESOLUTION_BITS))
#define VOLTAGE_TO_DAC(volt) ((uint16_t)((volt) * (1 << DAC_RESOLUTION_BITS) / DAC_REFERENCE_VOLTAGE))

// Timing helper macros
#define MS_TO_US(ms) ((ms) * 1000)
#define US_TO_MS(us) ((us) / 1000)
#define SEC_TO_MS(sec) ((sec) * 1000)
#define MIN_TO_MS(min) ((min) * 60 * 1000)

#ifdef __cplusplus
}
#endif

#endif // STM32_NUCLEO_F446RE_BOARD_CONFIG_H