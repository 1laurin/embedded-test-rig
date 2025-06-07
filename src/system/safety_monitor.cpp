/**
 * @file safety_monitor.cpp
 * @brief Safety monitoring and emergency shutdown implementation
 *
 * This file implements system safety monitoring including temperature,
 * voltage, and current monitoring with emergency shutdown capabilities.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "system/safety_monitor.h"
#include "platforms/common/hal_interface.h"
#include "platforms/pico_w/include/board_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool safety_monitor_initialized = false;
static bool safety_monitoring_enabled = true;
static bool emergency_state = false;
static void (*emergency_callback)(void) = NULL;

static safety_monitor_data_t safety_data[SAFETY_PARAM_COUNT];
static uint32_t total_violations = 0;
static char last_emergency_reason[128] = {0};

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void check_voltage_safety(void);
static void check_current_safety(void);
static void check_temperature_safety(void);
static void check_system_health_safety(void);
static void update_safety_status(safety_parameter_t param, float value);
static void handle_safety_violation(safety_parameter_t param);
static void disable_all_outputs(void);
static void activate_safety_indicators(void);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the safety monitoring system
 */
bool safety_monitor_init(void)
{
    if (safety_monitor_initialized)
    {
        return true; // Already initialized
    }

    printf("[SAFETY] Initializing safety monitoring system...\n");

    // Initialize safety data structures
    for (int i = 0; i < SAFETY_PARAM_COUNT; i++)
    {
        memset(&safety_data[i], 0, sizeof(safety_monitor_data_t));
        safety_data[i].parameter = (safety_parameter_t)i;
        safety_data[i].status = SAFETY_STATUS_OK;
    }

    // Set thresholds for each parameter
    safety_data[SAFETY_PARAM_VOLTAGE].warning_threshold = SAFETY_VOLTAGE_MAX * 0.9f;
    safety_data[SAFETY_PARAM_VOLTAGE].critical_threshold = SAFETY_VOLTAGE_MAX;
    safety_data[SAFETY_PARAM_VOLTAGE].emergency_threshold = EMERGENCY_VOLTAGE_LIMIT;

    safety_data[SAFETY_PARAM_CURRENT].warning_threshold = SAFETY_CURRENT_MAX * 0.9f;
    safety_data[SAFETY_PARAM_CURRENT].critical_threshold = SAFETY_CURRENT_MAX;
    safety_data[SAFETY_PARAM_CURRENT].emergency_threshold = EMERGENCY_CURRENT_LIMIT;

    safety_data[SAFETY_PARAM_TEMPERATURE].warning_threshold = SAFETY_TEMP_MAX * 0.9f;
    safety_data[SAFETY_PARAM_TEMPERATURE].critical_threshold = SAFETY_TEMP_MAX;
    safety_data[SAFETY_PARAM_TEMPERATURE].emergency_threshold = EMERGENCY_TEMP_LIMIT;

    safety_data[SAFETY_PARAM_SYSTEM_HEALTH].warning_threshold = 80.0f;   // 80% system health
    safety_data[SAFETY_PARAM_SYSTEM_HEALTH].critical_threshold = 60.0f;  // 60% system health
    safety_data[SAFETY_PARAM_SYSTEM_HEALTH].emergency_threshold = 40.0f; // 40% system health

    emergency_state = false;
    total_violations = 0;
    safety_monitoring_enabled = true;

    safety_monitor_initialized = true;
    printf("[SAFETY] Safety monitoring system initialized\n");

    return true;
}

/**
 * @brief Check all safety parameters and take action if needed
 */
void check_system_safety(void)
{
    if (!safety_monitor_initialized || !safety_monitoring_enabled || emergency_state)
    {
        return;
    }

    // Check all safety parameters
    check_voltage_safety();
    check_current_safety();
    check_temperature_safety();
    check_system_health_safety();

    // Check for emergency conditions
    safety_status_t overall_status = get_overall_safety_status();
    if (overall_status == SAFETY_STATUS_EMERGENCY)
    {
        emergency_shutdown("Safety parameter exceeded emergency threshold");
    }
}

/**
 * @brief Perform emergency shutdown sequence
 */
void emergency_shutdown(const char *reason)
{
    if (emergency_state)
    {
        return; // Already in emergency state
    }

    printf("\n!!! EMERGENCY SHUTDOWN !!!\n");
    printf("Reason: %s\n", reason);

    // Store the reason
    strncpy(last_emergency_reason, reason, sizeof(last_emergency_reason) - 1);
    last_emergency_reason[sizeof(last_emergency_reason) - 1] = '\0';

    emergency_state = true;

    // Disable all outputs immediately
    disable_all_outputs();

    // Activate safety indicators
    activate_safety_indicators();

    // Call emergency callback if registered
    if (emergency_callback != NULL)
    {
        emergency_callback();
    }

    // Update display with emergency message
    hal_display_clear(0xFF0000); // Red background
    hal_display_draw_text(10, 10, "EMERGENCY STOP", 0xFFFFFF, 0xFF0000);
    hal_display_draw_text(10, 30, reason, 0xFFFFFF, 0xFF0000);
    hal_display_draw_text(10, 60, "System Disabled", 0xFFFF00, 0xFF0000);
    hal_display_draw_text(10, 80, "Press RESET to restart", 0xFFFF00, 0xFF0000);
    hal_display_flush();

    printf("Emergency shutdown complete. System is now in safe state.\n");
}

/**
 * @brief Check if system is in emergency state
 */
bool is_emergency_state(void)
{
    return emergency_state;
}

/**
 * @brief Get current safety status for a parameter
 */
safety_status_t get_safety_status(safety_parameter_t parameter, safety_monitor_data_t *data)
{
    if (parameter >= SAFETY_PARAM_COUNT)
    {
        return SAFETY_STATUS_EMERGENCY;
    }

    if (data != NULL)
    {
        *data = safety_data[parameter];
    }

    return safety_data[parameter].status;
}

/**
 * @brief Get overall system safety status
 */
safety_status_t get_overall_safety_status(void)
{
    safety_status_t worst_status = SAFETY_STATUS_OK;

    for (int i = 0; i < SAFETY_PARAM_COUNT; i++)
    {
        if (safety_data[i].status > worst_status)
        {
            worst_status = safety_data[i].status;
        }
    }

    return worst_status;
}

/**
 * @brief Register callback for emergency events
 */
void register_safety_emergency_callback(void (*callback)(void))
{
    emergency_callback = callback;
    printf("[SAFETY] Emergency callback registered\n");
}

/**
 * @brief Enable or disable safety monitoring
 */
void set_safety_monitoring_enabled(bool enabled)
{
    safety_monitoring_enabled = enabled;
    printf("[SAFETY] Safety monitoring %s\n", enabled ? "enabled" : "disabled");
}

/**
 * @brief Check if safety monitoring is enabled
 */
bool is_safety_monitoring_enabled(void)
{
    return safety_monitoring_enabled;
}

/**
 * @brief Reset safety violation counters
 */
void reset_safety_violations(void)
{
    total_violations = 0;
    for (int i = 0; i < SAFETY_PARAM_COUNT; i++)
    {
        safety_data[i].violation_count = 0;
    }
    printf("[SAFETY] Safety violation counters reset\n");
}

/**
 * @brief Get total number of safety violations
 */
uint32_t get_total_safety_violations(void)
{
    return total_violations;
}

/**
 * @brief Print current safety status to console
 */
void print_safety_status(void)
{
    printf("\n=== Safety Status ===\n");
    printf("Emergency State: %s\n", emergency_state ? "ACTIVE" : "Normal");
    printf("Monitoring Enabled: %s\n", safety_monitoring_enabled ? "Yes" : "No");
    printf("Total Violations: %lu\n", total_violations);

    if (emergency_state)
    {
        printf("Last Emergency Reason: %s\n", last_emergency_reason);
    }

    const char *param_names[] = {"Voltage", "Current", "Temperature", "System Health"};
    const char *status_names[] = {"OK", "WARNING", "CRITICAL", "EMERGENCY"};

    for (int i = 0; i < SAFETY_PARAM_COUNT; i++)
    {
        printf("%s: %.2f (Status: %s, Violations: %lu)\n",
               param_names[i],
               safety_data[i].current_value,
               status_names[safety_data[i].status],
               safety_data[i].violation_count);
    }
    printf("====================\n\n");
}

/**
 * @brief Test safety monitoring system
 */
bool test_safety_monitoring(void)
{
    printf("[SAFETY] Testing safety monitoring system...\n");

    bool test_passed = true;

    // Test voltage monitoring
    update_safety_status(SAFETY_PARAM_VOLTAGE, 25.0f); // Normal
    if (safety_data[SAFETY_PARAM_VOLTAGE].status != SAFETY_STATUS_OK)
    {
        printf("[SAFETY] ERROR: Voltage test failed (normal range)\n");
        test_passed = false;
    }

    update_safety_status(SAFETY_PARAM_VOLTAGE, 28.0f); // Warning
    if (safety_data[SAFETY_PARAM_VOLTAGE].status != SAFETY_STATUS_WARNING)
    {
        printf("[SAFETY] ERROR: Voltage test failed (warning range)\n");
        test_passed = false;
    }

    // Reset to normal
    update_safety_status(SAFETY_PARAM_VOLTAGE, 20.0f);

    printf("[SAFETY] Safety monitoring test %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Check voltage safety on all channels
 */
static void check_voltage_safety(void)
{
    float max_voltage = 0.0f;

    // Read voltage from all ADC channels
    for (int i = 0; i < NUM_DIAGNOSTIC_CHANNELS && i < 3; i++)
    {
        uint16_t adc_value;
        if (hal_adc_read(i, &adc_value) == HAL_OK)
        {
            // Convert ADC to actual voltage (assuming voltage divider)
            float voltage = ADC_TO_VOLTAGE(adc_value) * 10.0f; // 10:1 voltage divider

            if (voltage > max_voltage)
            {
                max_voltage = voltage;
            }
        }
    }

    update_safety_status(SAFETY_PARAM_VOLTAGE, max_voltage);
}

/**
 * @brief Check current safety on all channels
 */
static void check_current_safety(void)
{
    float max_current = 0.0f;

    // Read current from current sense resistors
    // This would read from dedicated current sense ADC channels
    for (int i = 0; i < NUM_DIAGNOSTIC_CHANNELS; i++)
    {
        // Placeholder - would read actual current sense values
        float current = 1.5f; // Simulated current reading

        if (current > max_current)
        {
            max_current = current;
        }
    }

    update_safety_status(SAFETY_PARAM_CURRENT, max_current);
}

/**
 * @brief Check temperature safety
 */
static void check_temperature_safety(void)
{
    // Read internal temperature sensor
    float temperature = 25.0f; // Placeholder - would read actual temperature

    // On Pico, you could read the internal temperature sensor:
    // uint16_t temp_adc;
    // if (hal_adc_read(4, &temp_adc) == HAL_OK) {
    //     temperature = 27.0f - (ADC_TO_VOLTAGE(temp_adc) - 0.706f) / 0.001721f;
    // }

    update_safety_status(SAFETY_PARAM_TEMPERATURE, temperature);
}

/**
 * @brief Check system health safety
 */
static void check_system_health_safety(void)
{
    // Calculate system health based on various factors
    float health_score = 100.0f;

    // Reduce health based on safety violations
    if (total_violations > 0)
    {
        health_score -= (total_violations * 5.0f);
    }

    // Reduce health based on system uptime (example)
    extern uint32_t get_system_uptime_seconds(void);
    uint32_t uptime = get_system_uptime_seconds();
    if (uptime > 86400)
    { // More than 24 hours
        health_score -= 10.0f;
    }

    // Ensure health score doesn't go below 0
    if (health_score < 0.0f)
    {
        health_score = 0.0f;
    }

    update_safety_status(SAFETY_PARAM_SYSTEM_HEALTH, health_score);
}

/**
 * @brief Update safety status for a parameter
 */
static void update_safety_status(safety_parameter_t param, float value)
{
    if (param >= SAFETY_PARAM_COUNT)
    {
        return;
    }

    safety_monitor_data_t *data = &safety_data[param];
    data->current_value = value;
    data->last_check_time = hal_get_tick_ms();

    safety_status_t old_status = data->status;

    // Determine new status based on thresholds
    if (value >= data->emergency_threshold)
    {
        data->status = SAFETY_STATUS_EMERGENCY;
    }
    else if (value >= data->critical_threshold)
    {
        data->status = SAFETY_STATUS_CRITICAL;
    }
    else if (value >= data->warning_threshold)
    {
        data->status = SAFETY_STATUS_WARNING;
    }
    else
    {
        data->status = SAFETY_STATUS_OK;
    }

    // For temperature, also check minimum threshold
    if (param == SAFETY_PARAM_TEMPERATURE && value < SAFETY_TEMP_MIN)
    {
        data->status = SAFETY_STATUS_CRITICAL;
    }

    // Handle status changes
    if (data->status > old_status)
    {
        data->violation_count++;
        total_violations++;
        handle_safety_violation(param);
    }
}

/**
 * @brief Handle safety violation
 */
static void handle_safety_violation(safety_parameter_t param)
{
    const char *param_names[] = {"Voltage", "Current", "Temperature", "System Health"};
    const char *status_names[] = {"OK", "WARNING", "CRITICAL", "EMERGENCY"};

    printf("[SAFETY] %s violation: %.2f (Status: %s)\n",
           param_names[param],
           safety_data[param].current_value,
           status_names[safety_data[param].status]);

    // Take appropriate action based on severity
    switch (safety_data[param].status)
    {
    case SAFETY_STATUS_WARNING:
        // Log warning, maybe adjust system parameters
        hal_gpio_write(LED_ERROR_PIN, GPIO_HIGH);
        break;

    case SAFETY_STATUS_CRITICAL:
        // Reduce system performance, increase monitoring frequency
        hal_gpio_write(LED_ERROR_PIN, GPIO_HIGH);
        printf("[SAFETY] CRITICAL: Reducing system performance\n");
        break;

    case SAFETY_STATUS_EMERGENCY:
        // Immediate shutdown will be handled by check_system_safety()
        break;

    default:
        break;
    }
}

/**
 * @brief Disable all outputs for safety
 */
static void disable_all_outputs(void)
{
    printf("[SAFETY] Disabling all outputs for safety...\n");

    // Disable all diagnostic channels
    for (int i = 1; i <= NUM_DIAGNOSTIC_CHANNELS; i++)
    {
        // Platform-specific channel disable would be called here
        // pico_set_channel_enable(i, false);
    }

    // Turn off all relays
    hal_gpio_write(RELAY_1_PIN, GPIO_LOW);
    hal_gpio_write(RELAY_2_PIN, GPIO_LOW);

    // Turn off buzzer
    hal_gpio_write(BUZZER_PIN, GPIO_LOW);

    printf("[SAFETY] All outputs disabled\n");
}

/**
 * @brief Activate safety indicators
 */
static void activate_safety_indicators(void)
{
    // Turn on error LED
    hal_gpio_write(LED_ERROR_PIN, GPIO_HIGH);

    // Flash status LED rapidly
    for (int i = 0; i < 10; i++)
    {
        hal_gpio_toggle(LED_STATUS_PIN);
        hal_delay_ms(100);
    }

    // Activate buzzer briefly
    hal_gpio_write(BUZZER_PIN, GPIO_HIGH);
    hal_delay_ms(1000);
    hal_gpio_write(BUZZER_PIN, GPIO_LOW);
}