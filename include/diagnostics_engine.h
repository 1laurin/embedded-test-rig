/**
 * @file diagnostics_engine.h
 * @brief Diagnostics engine interface with all required functions
 */

#ifndef DIAGNOSTICS_ENGINE_H
#define DIAGNOSTICS_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Core diagnostic functions
void toggle_all_channels(void);
void test_diagnostic_channels(void);
void get_channel_states(bool* states);
void set_channel_enable(int channel, bool enable);
void print_system_status(void);

// Diagnostic engine lifecycle
bool diagnostics_engine_init(void);
void diagnostics_engine_deinit(void);

// Channel management
void enable_all_channels(void);
void disable_all_channels(void);
bool is_channel_enabled(int channel);

// Status and monitoring
void update_channel_status(void);
void run_channel_diagnostics(void);

#ifdef __cplusplus
}
#endif

#endif // DIAGNOSTICS_ENGINE_H
