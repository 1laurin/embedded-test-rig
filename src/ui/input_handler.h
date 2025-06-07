/**
 * @file input_handler.h
 * @brief User input handling interface
 *
 * This file provides the interface for handling user inputs such as
 * button presses, rotary encoder, touch screen, or remote commands.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // CONFIGURATION CONSTANTS
    // =============================================================================

#define BUTTON_DEBOUNCE_MS 50
#define LONG_PRESS_DURATION_MS 2000
#define DOUBLE_CLICK_WINDOW_MS 500

    // =============================================================================
    // INPUT EVENT TYPES
    // =============================================================================

    typedef enum
    {
        INPUT_EVENT_NONE = 0,
        INPUT_EVENT_BUTTON_PRESS,
        INPUT_EVENT_BUTTON_RELEASE,
        INPUT_EVENT_BUTTON_LONG_PRESS,
        INPUT_EVENT_BUTTON_DOUBLE_CLICK,
        INPUT_EVENT_UART_COMMAND,
        INPUT_EVENT_EMERGENCY_STOP
    } input_event_type_t;

    typedef enum
    {
        BUTTON_USER = 0,
        BUTTON_RESET,
        BUTTON_MODE,
        BUTTON_EMERGENCY,
        BUTTON_COUNT
    } button_id_t;

    typedef struct
    {
        input_event_type_t type;
        button_id_t button_id;
        uint32_t timestamp;
        union
        {
            struct
            {
                bool state;           // true = pressed, false = released
                uint32_t duration_ms; // For long press events
            } button;
            struct
            {
                char command[64]; // UART command string
                uint8_t uart_id;  // Which UART received the command
            } uart;
        } data;
    } input_event_t;

    // =============================================================================
    // PUBLIC FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize the input handler subsystem
     * @return true on success, false on failure
     */
    bool input_handler_init(void);

    /**
     * @brief Process all pending user inputs
     * This function should be called regularly from the main loop
     */
    void handle_user_input(void);

    /**
     * @brief Get the next pending input event
     * @param event Pointer to store the event data
     * @return true if event available, false if queue empty
     */
    bool get_next_input_event(input_event_t *event);

    /**
     * @brief Check if there are pending input events
     * @return Number of pending events in the queue
     */
    uint8_t get_pending_input_count(void);

    /**
     * @brief Clear all pending input events
     */
    void clear_input_events(void);

    /**
     * @brief Callback for button interrupt (called from HAL)
     * @param pin GPIO pin that triggered the interrupt
     */
    void user_button_callback(uint32_t pin);

    /**
     * @brief Process UART command input
     * @param uart_id UART instance that received data
     * @param command Command string received
     */
    void process_uart_command(uint8_t uart_id, const char *command);

    /**
     * @brief Enable or disable input processing
     * @param enabled true to enable, false to disable
     */
    void set_input_processing_enabled(bool enabled);

    /**
     * @brief Check if input processing is enabled
     * @return true if enabled, false if disabled
     */
    bool is_input_processing_enabled(void);

    /**
     * @brief Register emergency stop callback
     * @param callback Function to call when emergency stop is triggered
     */
    void register_emergency_stop_callback(void (*callback)(void));

    // =============================================================================
    // BUTTON STATE QUERY FUNCTIONS
    // =============================================================================

    /**
     * @brief Get current state of a button
     * @param button_id Button to query
     * @return true if pressed, false if released
     */
    bool get_button_state(button_id_t button_id);

    /**
     * @brief Check if button was just pressed (edge detection)
     * @param button_id Button to check
     * @return true if button was just pressed
     */
    bool was_button_pressed(button_id_t button_id);

    /**
     * @brief Check if button was just released (edge detection)
     * @param button_id Button to check
     * @return true if button was just released
     */
    bool was_button_released(button_id_t button_id);

    /**
     * @brief Get button press duration
     * @param button_id Button to check
     * @return Duration in milliseconds (0 if not pressed)
     */
    uint32_t get_button_press_duration(button_id_t button_id);

#ifdef __cplusplus
}
#endif

#endif // INPUT_HANDLER_H