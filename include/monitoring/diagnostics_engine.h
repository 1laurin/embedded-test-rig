/**
 * @file diagnostics_engine.h
 * @brief Diagnostics engine interface
 */

#ifndef DIAGNOSTICS_ENGINE_H
#define DIAGNOSTICS_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void toggle_all_channels(void);
void test_diagnostic_channels(void);
void get_channel_states(bool* states);
void set_channel_enable(int channel, bool enable);
void print_system_status(void);

#ifdef __cplusplus
}
#endif

#endif // DIAGNOSTICS_ENGINE_H
