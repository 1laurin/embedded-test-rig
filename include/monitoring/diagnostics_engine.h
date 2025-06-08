/**
 * @file diagnostics_engine.h
 * @brief Diagnostic engine interface
 */

#ifndef DIAGNOSTICS_ENGINE_H
#define DIAGNOSTICS_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NUM_DIAGNOSTIC_CHANNELS
#define NUM_DIAGNOSTIC_CHANNELS 4
#endif

bool diagnostics_engine_init(void);
void diagnostics_engine_deinit(void);
void toggle_all_channels(void);
void test_diagnostic_channels(void);
void get_channel_states(bool* channels);
void set_channel_enable(int channel, bool enable);
void print_system_status(void);

#ifdef __cplusplus
}
#endif

#endif // DIAGNOSTICS_ENGINE_H
