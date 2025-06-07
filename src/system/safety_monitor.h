/**
 * @file safety_monitor.h
 * @brief Safety monitoring and emergency shutdown interface
 *
 * This file provides the interface for monitoring system safety parameters
 * and implementing emergency shutdown procedures.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

// =============================================================================
// SAFETY PARAMETER THRESHOLDS
// =============================================================================

// These would typically come from board_config.h, but defined here for clarity
#ifndef SAFETY_VOLTAGE_MAX
#define SAFETY_VOLTAGE_MAX 30.0f
#endif

#ifndef SAFETY_CURRENT_MAX
#define SAFETY_CURRENT_MAX 10.0f
#endif

#ifndef SAFETY_TEMP_MAX
#define SAFETY_TEMP_MAX 85.0f
#endif

#ifndef SAFETY_TEMP_MIN
#define SAFETY_TEMP_MIN -10.0f
#endif

#ifndef EMERGENCY_VOLTAGE_LIMIT
#define EMERGENCY_VOLTAGE_LIMIT 35.0f
#endif

#ifndef EMERGENCY_CURRENT_LIMIT
#define EMERGENCY_CURRENT_LIMIT 12.0f
#endif

#ifndef EMERGENCY_TEMP_LIMIT
#define EMERGENCY_TEMP_LIMIT 95.0f
#endif

    // =============================================================================
    // SAFETY STATUS TYPES
    // =============================================================================

    typedef enum
    {
        SAFETY_STATUS_OK = 0,
        SAFETY_STATUS_WARNING,
        SAFETY_STATUS_CRITICAL,
        SAFETY_STATUS_EMERGENCY
    } safety_status_t;

    typedef enum
    {
        SAFETY_PARAM_VOLTAGE = 0,
        SAFETY_PARAM_CURRENT,
        SAFETY_PARAM_TEMPERATURE,
        SAFETY_PARAM_SYSTEM_HEALTH,
        SAFETY_PARAM_COUNT
    } safety_parameter_t;

    typedef struct
    {
        safety_parameter_t parameter;
        float current_value;
        float warning_threshold;
        float critical_threshold;
        float emergency_threshold;
        safety_status_t status;
        uint32_t last_check_time;
        uint32_t violation_count;
    } safety_monitor_data_t;

    // =============================================================================
    // PUBLIC FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize the safety monitoring system
     * @return true on success, false on failure
     */
    bool safety_monitor_init(void);

    /**
     * @brief Check all safety parameters and take action if needed
     * This function should be called regularly from the main loop
     */
    void check_system_safety(void);

    /**
     * @brief Perform emergency shutdown sequence with a reason
     * @param reason String describing the reason for emergency shutdown
     */
    void emergency_shutdown(const char *reason);

    /**
     * @brief Check if system is in emergency state
     * @return true if emergency shutdown is active
     */
    bool is_emergency_state(void);

    /**
     * @brief Get current safety status for a parameter
     * @param parameter Safety parameter to check
     * @param data Pointer to store safety data (can be NULL)
     * @return Current safety status
     */
    safety_status_t get_safety_status(safety_parameter_t parameter, safety_monitor_data_t *data);

    /**
     * @brief Get overall system safety status
     * @return Worst case safety status across all parameters
     */
    safety_status_t get_overall_safety_status(void);

    /**
     * @brief Register callback for emergency events
     * @param callback Function to call when emergency shutdown occurs
     */
    void register_safety_emergency_callback(void (*callback)(void));

    /**
     * @brief Enable or disable safety monitoring
     * @param enabled true to enable, false to disable
     */
    void set_safety_monitoring_enabled(bool enabled);

    /**
     * @brief Check if safety monitoring is enabled
     * @return true if enabled, false if disabled
     */
    bool is_safety_monitoring_enabled(void);

    /**
     * @brief Reset safety violation counters
     */
    void reset_safety_violations(void);

    /**
     * @brief Get total number of safety violations
     * @return Total violation count across all parameters
     */
    uint32_t get_total_safety_violations(void);

    /**
     * @brief Print current safety status to console
     */
    void print_safety_status(void);

    /**
     * @brief Test safety monitoring system
     * @return true if test passed, false if failed
     */
    bool test_safety_monitoring(void);

#ifdef __cplusplus
}
#endif

#endif // SAFETY_MONITOR_H