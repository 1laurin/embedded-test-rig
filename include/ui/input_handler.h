#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H
#include <stdbool.h>
#include <stdint.h>
bool input_handler_init(void);
void handle_user_input(void);
void user_button_callback(uint32_t pin);
void register_emergency_stop_callback(void (*callback)(void));
void set_input_processing_enabled(bool enabled);
void clear_input_events(void);
#endif
