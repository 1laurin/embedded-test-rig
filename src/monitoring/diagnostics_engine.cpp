/**
 * @file diagnostics_engine.cpp
 * @brief Diagnostics engine implementation with all required functions
 */

#include "diagnostics_engine.h"
#include "hal_interface.h"
#include "board_config.h"
#include <stdio.h>

static bool diagnostics_initialized = false;
static bool channels_enabled[4] = {false, false, false, false};

bool diagnostics_engine_init(void) {
    printf("[DIAG] Initializing diagnostics engine...\n");
    diagnostics_initialized = true;
    return true;
}

void diagnostics_engine_deinit(void) {
    printf("[DIAG] Deinitializing diagnostics engine...\n");
    diagnostics_initialized = false;
}

void toggle_all_channels(void) {
    printf("[DIAG] Toggling all diagnostic channels\n");
    for (int i = 0; i < 4; i++) {
        channels_enabled[i] = !channels_enabled[i];
        printf("[DIAG] Channel %d: %s\n", i+1, channels_enabled[i] ? "ON" : "OFF");
    }
}

void test_diagnostic_channels(void) {
    if (!diagnostics_initialized) return;
    
    printf("[DIAG] Testing diagnostic channels...\n");
    
    // Test each enabled channel
    for (int i = 0; i < 4; i++) {
        if (channels_enabled[i]) {
            printf("[DIAG] Testing channel %d...\n", i+1);
            
            // Read ADC if available
            uint16_t adc_value;
            if (i < 3 && hal_adc_read(i, &adc_value) == HAL_OK) {
                float voltage = (float)adc_value * 3.3f / 4095.0f;
                printf("[DIAG] Channel %d voltage: %.3f V\n", i+1, voltage);
            }
        }
    }
}

void get_channel_states(bool* states) {
    for (int i = 0; i < 4; i++) {
        states[i] = channels_enabled[i];
    }
}

void set_channel_enable(int channel, bool enable) {
    if (channel >= 1 && channel <= 4) {
        channels_enabled[channel-1] = enable;
        printf("[DIAG] Channel %d %s\n", channel, enable ? "enabled" : "disabled");
    }
}

void enable_all_channels(void) {
    printf("[DIAG] Enabling all channels\n");
    for (int i = 0; i < 4; i++) {
        channels_enabled[i] = true;
    }
}

void disable_all_channels(void) {
    printf("[DIAG] Disabling all channels\n");
    for (int i = 0; i < 4; i++) {
        channels_enabled[i] = false;
    }
}

bool is_channel_enabled(int channel) {
    if (channel >= 1 && channel <= 4) {
        return channels_enabled[channel-1];
    }
    return false;
}

void update_channel_status(void) {
    // Update channel status based on hardware readings
    for (int i = 0; i < 4; i++) {
        if (channels_enabled[i]) {
            // Could add status monitoring here
        }
    }
}

void run_channel_diagnostics(void) {
    test_diagnostic_channels();
}

void print_system_status(void) {
    printf("[STATUS] Diagnostic System Status:\n");
    printf("[STATUS] Engine initialized: %s\n", diagnostics_initialized ? "Yes" : "No");
    
    for (int i = 0; i < 4; i++) {
        printf("[STATUS] Channel %d: %s\n", i+1, channels_enabled[i] ? "ENABLED" : "DISABLED");
    }
    
    // Print uptime if available
    extern uint32_t get_system_uptime_seconds(void);
    printf("[STATUS] System uptime: %lu seconds\n", get_system_uptime_seconds());
}
