/**
 * @file input_handler.cpp
 * @brief User input handling implementation
 *
 * This file implements user input processing including button debouncing,
 * long press detection, and command processing.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "ui/input_handler.h"
#include "monitoring/diagnostics_engine.h"
#include "platforms/common/hal_interface.h"
#include "platforms/pico_w/include/board_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// PRIVATE CONSTANTS
// =============================================================================

#define INPUT_EVENT_QUEUE_SIZE 16
#define MAX_UART_COMMAND_LENGTH 64

// =============================================================================
// PRIVATE TYPES
// =============================================================================

typedef struct
{
    bool current_state;
    bool previous_state;
    bool pressed_edge;
    bool released_edge;
    uint32_t press_start_time;
    uint32_t last_debounce_time;
    bool long_press_triggered;
    uint8_t click_count;
    uint32_t last_click_time;
} button_state_t;

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool input_handler_initialized = false;
static bool input_processing_enabled = true;
static volatile bool user_button_pressed = false;

static button_state_t button_states[BUTTON_COUNT] = {0};
static input_event_t event_queue[INPUT_EVENT_QUEUE_SIZE];
static volatile uint8_t queue_head = 0;
static volatile uint8_t queue_tail = 0;
static volatile uint8_t queue_count = 0;

static void (*emergency_stop_callback)(void) = NULL;

// GPIO pin mapping for buttons
static const uint32_t button_pins[BUTTON_COUNT] = {
    BTN_USER_PIN,  // BUTTON_USER
    BTN_RESET_PIN, // BUTTON_RESET
    BTN_MODE_PIN,  // BUTTON_MODE
    EXT_INT_PIN    // BUTTON_EMERGENCY (using external interrupt pin)
};

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void update_button_states(void);
static void process_button_events(void);
static bool add_input_event(const input_event_t *event);
static void handle_button_press(button_id_t button_id);
static void handle_button_release(button_id_t button_id);
static void handle_button_long_press(button_id_t button_id);
static void handle_button_double_click(button_id_t button_id);
static bool read_button_gpio(button_id_t button_id);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the input handler subsystem
 */
bool input_handler_init(void)
{
    if (input_handler_initialized)
    {
        return true; // Already initialized
    }

    printf("[INPUT] Initializing input handler...\n");

    // Initialize button states
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        memset(&button_states[i], 0, sizeof(button_state_t));
    }

    // Clear event queue
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;

    // Initialize button GPIO pins (they should already be configured in hal_gpio_init)
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        gpio_state_t initial_state;
        if (hal_gpio_read(button_pins[i], &initial_state) == HAL_OK)
        {
            // Buttons are active low (pressed = 0)
            button_states[i].current_state = (initial_state == GPIO_LOW);
            button_states[i].previous_state = button_states[i].current_state;
        }
    }

    input_processing_enabled = true;
    input_handler_initialized = true;

    printf("[INPUT] Input handler initialized successfully\n");
    return true;
}

/**
 * @brief Process all pending user inputs
 */
void handle_user_input(void)
{
    if (!input_handler_initialized || !input_processing_enabled)
    {
        return;
    }

    // Handle interrupt-driven button press
    static uint32_t last_button_time = 0;
    uint32_t current_time = hal_get_tick_ms();

    if (user_button_pressed && (current_time - last_button_time) > BUTTON_DEBOUNCE_MS)
    {
        user_button_pressed = false;
        last_button_time = current_time;

        printf("[INPUT] User button pressed! Toggling diagnostic channels...\n");

        // Toggle all diagnostic channels
        toggle_all_channels();

        // Toggle communication LED
        static bool comm_led_state = false;
        comm_led_state = !comm_led_state;
        hal_gpio_write(LED_COMM_PIN, comm_led_state ? GPIO_HIGH : GPIO_LOW);

        // Add button press event to queue
        input_event_t event = {0};
        event.type = INPUT_EVENT_BUTTON_PRESS;
        event.button_id = BUTTON_USER;
        event.timestamp = current_time;
        event.data.button.state = true;
        add_input_event(&event);
    }

    // Update button states and process events
    update_button_states();
    process_button_events();
}

/**
 * @brief Get the next pending input event
 */
bool get_next_input_event(input_event_t *event)
{
    if (!input_handler_initialized || event == NULL || queue_count == 0)
    {
        return false;
    }

    // Copy event from queue
    *event = event_queue[queue_tail];

    // Update queue pointers
    queue_tail = (queue_tail + 1) % INPUT_EVENT_QUEUE_SIZE;
    queue_count--;

    return true;
}

/**
 * @brief Check if there are pending input events
 */
uint8_t get_pending_input_count(void)
{
    return queue_count;
}

/**
 * @brief Clear all pending input events
 */
void clear_input_events(void)
{
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;
}

/**
 * @brief Callback for button interrupt (called from HAL)
 */
void user_button_callback(uint32_t pin)
{
    if (pin == BTN_USER_PIN)
    {
        user_button_pressed = true;
    }
    else if (pin == EXT_INT_PIN)
    {
        // Emergency stop button
        if (emergency_stop_callback != NULL)
        {
            emergency_stop_callback();
        }

        input_event_t event = {0};
        event.type = INPUT_EVENT_EMERGENCY_STOP;
        event.button_id = BUTTON_EMERGENCY;
        event.timestamp = hal_get_tick_ms();
        add_input_event(&event);
    }
}

/**
 * @brief Process UART command input
 */
void process_uart_command(uint8_t uart_id, const char *command)
{
    if (!input_handler_initialized || command == NULL)
    {
        return;
    }

    printf("[INPUT] UART%d command: %s\n", uart_id, command);

    input_event_t event = {0};
    event.type = INPUT_EVENT_UART_COMMAND;
    event.timestamp = hal_get_tick_ms();
    event.data.uart.uart_id = uart_id;
    strncpy(event.data.uart.command, command, sizeof(event.data.uart.command) - 1);
    event.data.uart.command[sizeof(event.data.uart.command) - 1] = '\0';

    add_input_event(&event);

    // Process common commands immediately
    if (strcmp(command, "STOP") == 0 || strcmp(command, "EMERGENCY") == 0)
    {
        if (emergency_stop_callback != NULL)
        {
            emergency_stop_callback();
        }
    }
    else if (strcmp(command, "STATUS") == 0)
    {
        print_system_status();
    }
    else if (strcmp(command, "RESET") == 0)
    {
        printf("[INPUT] Reset command received\n");
        // Implement reset logic here
    }
    else if (strncmp(command, "CHANNEL", 7) == 0)
    {
        // Handle channel control commands
        // Example: "CHANNEL 1 ON", "CHANNEL 2 OFF"
        int channel;
        char state[8];
        if (sscanf(command, "CHANNEL %d %7s", &channel, state) == 2)
        {
            bool enable = (strcmp(state, "ON") == 0);
            set_channel_enable(channel, enable);
            printf("[INPUT] Channel %d set to %s\n", channel, enable ? "ON" : "OFF");
        }
    }
}

/**
 * @brief Enable or disable input processing
 */
void set_input_processing_enabled(bool enabled)
{
    input_processing_enabled = enabled;
    printf("[INPUT] Input processing %s\n", enabled ? "enabled" : "disabled");
}

/**
 * @brief Check if input processing is enabled
 */
bool is_input_processing_enabled(void)
{
    return input_processing_enabled;
}

/**
 * @brief Register emergency stop callback
 */
void register_emergency_stop_callback(void (*callback)(void))
{
    emergency_stop_callback = callback;
    printf("[INPUT] Emergency stop callback registered\n");
}

/**
 * @brief Get current state of a button
 */
bool get_button_state(button_id_t button_id)
{
    if (button_id >= BUTTON_COUNT)
    {
        return false;
    }
    return button_states[button_id].current_state;
}

/**
 * @brief Check if button was just pressed (edge detection)
 */
bool was_button_pressed(button_id_t button_id)
{
    if (button_id >= BUTTON_COUNT)
    {
        return false;
    }
    bool result = button_states[button_id].pressed_edge;
    button_states[button_id].pressed_edge = false; // Clear edge flag
    return result;
}

/**
 * @brief Check if button was just released (edge detection)
 */
bool was_button_released(button_id_t button_id)
{
    if (button_id >= BUTTON_COUNT)
    {
        return false;
    }
    bool result = button_states[button_id].released_edge;
    button_states[button_id].released_edge = false; // Clear edge flag
    return result;
}

/**
 * @brief Get button press duration
 */
uint32_t get_button_press_duration(button_id_t button_id)
{
    if (button_id >= BUTTON_COUNT || !button_states[button_id].current_state)
    {
        return 0;
    }
    return hal_get_tick_ms() - button_states[button_id].press_start_time;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Update button states with debouncing
 */
static void update_button_states(void)
{
    uint32_t current_time = hal_get_tick_ms();

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        button_state_t *btn = &button_states[i];
        bool raw_state = read_button_gpio((button_id_t)i);

        // Debouncing logic
        if (raw_state != btn->current_state)
        {
            if (current_time - btn->last_debounce_time > BUTTON_DEBOUNCE_MS)
            {
                btn->previous_state = btn->current_state;
                btn->current_state = raw_state;
                btn->last_debounce_time = current_time;

                // Detect edges
                if (btn->current_state && !btn->previous_state)
                {
                    // Button pressed
                    btn->pressed_edge = true;
                    btn->press_start_time = current_time;
                    btn->long_press_triggered = false;

                    // Handle double-click detection
                    if (current_time - btn->last_click_time < DOUBLE_CLICK_WINDOW_MS)
                    {
                        btn->click_count++;
                    }
                    else
                    {
                        btn->click_count = 1;
                    }
                    btn->last_click_time = current_time;
                }
                else if (!btn->current_state && btn->previous_state)
                {
                    // Button released
                    btn->released_edge = true;
                }
            }
        }

        // Check for long press
        if (btn->current_state && !btn->long_press_triggered)
        {
            if (current_time - btn->press_start_time > LONG_PRESS_DURATION_MS)
            {
                btn->long_press_triggered = true;

                input_event_t event = {0};
                event.type = INPUT_EVENT_BUTTON_LONG_PRESS;
                event.button_id = (button_id_t)i;
                event.timestamp = current_time;
                event.data.button.duration_ms = current_time - btn->press_start_time;
                add_input_event(&event);
            }
        }

        // Check for double-click after release
        if (btn->released_edge && btn->click_count >= 2)
        {
            input_event_t event = {0};
            event.type = INPUT_EVENT_BUTTON_DOUBLE_CLICK;
            event.button_id = (button_id_t)i;
            event.timestamp = current_time;
            add_input_event(&event);

            btn->click_count = 0; // Reset click count
        }
    }
}

/**
 * @brief Process button events and add them to the queue
 */
static void process_button_events(void)
{
    uint32_t current_time = hal_get_tick_ms();

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        button_state_t *btn = &button_states[i];

        if (btn->pressed_edge)
        {
            handle_button_press((button_id_t)i);
            btn->pressed_edge = false;
        }

        if (btn->released_edge)
        {
            handle_button_release((button_id_t)i);
            btn->released_edge = false;
        }
    }
}

/**
 * @brief Add input event to the queue
 */
static bool add_input_event(const input_event_t *event)
{
    if (queue_count >= INPUT_EVENT_QUEUE_SIZE)
    {
        printf("[INPUT] WARNING: Event queue full, dropping event\n");
        return false;
    }

    event_queue[queue_head] = *event;
    queue_head = (queue_head + 1) % INPUT_EVENT_QUEUE_SIZE;
    queue_count++;

    return true;
}

/**
 * @brief Handle button press event
 */
static void handle_button_press(button_id_t button_id)
{
    input_event_t event = {0};
    event.type = INPUT_EVENT_BUTTON_PRESS;
    event.button_id = button_id;
    event.timestamp = hal_get_tick_ms();
    event.data.button.state = true;

    add_input_event(&event);

    // Handle specific button actions
    switch (button_id)
    {
    case BUTTON_USER:
        printf("[INPUT] User button pressed\n");
        break;
    case BUTTON_RESET:
        printf("[INPUT] Reset button pressed\n");
        break;
    case BUTTON_MODE:
        printf("[INPUT] Mode button pressed\n");
        break;
    case BUTTON_EMERGENCY:
        printf("[INPUT] Emergency button pressed\n");
        if (emergency_stop_callback != NULL)
        {
            emergency_stop_callback();
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Handle button release event
 */
static void handle_button_release(button_id_t button_id)
{
    input_event_t event = {0};
    event.type = INPUT_EVENT_BUTTON_RELEASE;
    event.button_id = button_id;
    event.timestamp = hal_get_tick_ms();
    event.data.button.state = false;
    event.data.button.duration_ms = hal_get_tick_ms() - button_states[button_id].press_start_time;

    add_input_event(&event);
}

/**
 * @brief Read button GPIO state
 */
static bool read_button_gpio(button_id_t button_id)
{
    if (button_id >= BUTTON_COUNT)
    {
        return false;
    }

    gpio_state_t state;
    if (hal_gpio_read(button_pins[button_id], &state) == HAL_OK)
    {
        // Buttons are active low (pressed = 0)
        return (state == GPIO_LOW);
    }

    return false;
}