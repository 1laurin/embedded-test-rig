/**
 * @file main.cpp
 * @brief Main application with enhanced WiFi and WebSocket integration for Raspberry Pi Pico W
 *
 * This file integrates WiFi connectivity and WebSocket server with the existing
 * diagnostic test rig functionality, enabling real-time web-based monitoring.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "system_init.h"
#include "system_loop.h"
#include "system_info.h"
#include "input_handler.h"
#include "safety_monitor.h"
#include "hal_demo.h"
#include "hal_test.h"
#include "wifi_manager.h"
#include "websocket_server.h"
#include "diagnostics_engine.h"
#include "board_config.h"

// Pico W specific includes
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include <stdio.h>
#include <string.h>

// =============================================================================
// WIFI CREDENTIALS (Change these for your network)
// =============================================================================

#ifndef WIFI_SSID
#define WIFI_SSID "YourWiFiNetwork"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YourWiFiPassword"
#endif

// Set to true to use the above credentials, false to configure via UART
#ifndef USE_HARDCODED_WIFI
#define USE_HARDCODED_WIFI false
#endif

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void print_startup_banner(void);
static void setup_emergency_stop(void);
static void system_emergency_stop_handler(void);
static void cleanup_and_exit(void);
static bool setup_wifi_connection(void);
// Forward declarations with simplified types
static void wifi_event_handler_simplified(void);
static bool websocket_command_handler(const char *command, const char *params, int client_id);
static void websocket_client_handler(int client_id, bool connected, const char *client_ip);
static void configure_wifi_via_uart(void);
static void send_channel_updates(void);
static void web_integration_update(void);
static void update_wifi_led_status(void);
static bool initialize_pico_w_hardware(void);
static void send_system_status_update(void);
static void handle_uart_wifi_commands(void);

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool wifi_setup_complete = false;
static bool websocket_setup_complete = false;
static bool pico_w_initialized = false;
static uint32_t last_channel_update = 0;
static uint32_t last_web_update = 0;
static uint32_t last_status_update = 0;
static uint32_t last_wifi_led_update = 0;
static uint32_t wifi_connection_attempts = 0;
static bool wifi_led_state = false;

// WiFi configuration buffer for UART commands
static char wifi_ssid_buffer[WIFI_SSID_MAX_LENGTH] = {0};
static char wifi_password_buffer[WIFI_PASSWORD_MAX_LENGTH] = {0};
static bool wifi_configured_via_uart = false;

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main()
{
    // Initialize stdio for printf/scanf
    stdio_init_all();

    // Small delay to allow USB connection to stabilize
    sleep_ms(1000);

    // Print startup banner
    print_startup_banner();

    // Initialize Pico W specific hardware first
    printf("[MAIN] Initializing Pico W hardware...\n");
    if (!initialize_pico_w_hardware())
    {
        printf("CRITICAL ERROR: Pico W hardware initialization failed\n");
        return -1;
    }

    // Initialize the entire system
    printf("[MAIN] Initializing core system...\n");
    hal_status_t init_status = system_init();
    if (init_status != HAL_OK)
    {
        printf("CRITICAL ERROR: System initialization failed (status: %d)\n", init_status);
        printf("System cannot continue. Please check hardware connections.\n");
        cleanup_and_exit();
        return -1;
    }

    // Initialize input handler
    if (!input_handler_init())
    {
        printf("ERROR: Input handler initialization failed\n");
        cleanup_and_exit();
        return -2;
    }

    // Setup emergency stop handling
    setup_emergency_stop();

    // Print system information
    display_system_info();
    print_init_progress();

    // Initialize WiFi and WebSocket
    printf("[MAIN] Setting up network connectivity...\n");
    if (!setup_wifi_connection())
    {
        printf("WARNING: WiFi setup failed, continuing without network features\n");
        if (websocket_setup_complete)
        {
            websocket_send_log("warn", "Network", "WiFi connection failed - running in offline mode");
        }
    }

    // Run HAL feature demonstration
    printf("[MAIN] Running HAL demonstration...\n");
    run_hal_demo();
    if (websocket_setup_complete)
    {
        websocket_send_log("info", "HAL", "HAL demonstration completed");
    }

    // Test all HAL subsystems
    printf("[MAIN] Testing HAL subsystems...\n");
    if (!test_hal_subsystems())
    {
        printf("WARNING: Some HAL subsystem tests failed\n");
        if (websocket_setup_complete)
        {
            websocket_send_log("warn", "HAL", "Some HAL subsystem tests failed");
        }
    }
    else
    {
        if (websocket_setup_complete)
        {
            websocket_send_log("info", "HAL", "All HAL subsystem tests passed");
        }
    }

    printf("\n");
    printf("=======================================================\n");
    printf(" System Ready - Starting Main Application Loop\n");
    printf("=======================================================\n");

    if (wifi_is_connected())
    {
        printf("📡 WiFi Connected: %s\n", wifi_get_ip_address());
        printf("🌐 WebSocket Server: ws://%s:%d\n", wifi_get_ip_address(), NET_WEBSOCKET_PORT);
        printf("🖥️  Web Interface: http://%s:%d\n", wifi_get_ip_address(), NET_HTTP_PORT);
    }
    else
    {
        printf("📡 WiFi: Not connected (offline mode)\n");
        if (!USE_HARDCODED_WIFI)
        {
            printf("💡 Send 'WIFI_CONNECT <SSID> <PASSWORD>' via UART to connect\n");
        }
    }

    printf("🎛️  Press user button to toggle diagnostic channels\n");
    printf("📡 Send UART commands for remote control\n");
    printf("🛡️  System performing automatic safety monitoring\n");
    printf("\n");

    if (websocket_setup_complete)
    {
        websocket_send_log("info", "System", "Multi-Channel Diagnostic Test Rig online and ready");
    }

    // Enter the main application loop
    run_main_loop();

    // If we reach here, the system is shutting down
    cleanup_and_exit();
    return 0;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Initialize Pico W specific hardware
 */
static bool initialize_pico_w_hardware(void)
{
    // Initialize CYW43 WiFi/Bluetooth chip
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA) != 0)
    {
        printf("[PICO_W] Failed to initialize CYW43 WiFi chip\n");
        return false;
    }

    // Enable WiFi station mode
    cyw43_arch_enable_sta_mode();

    // Set the WiFi LED to off initially
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    pico_w_initialized = true;
    printf("[PICO_W] CYW43 WiFi chip initialized successfully\n");
    return true;
}

/**
 * @brief Print the startup banner
 */
static void print_startup_banner(void)
{
    printf("\n");
    printf("=========================================================\n");
    printf("  Multi-Channel Diagnostic Test Rig - Pico W Edition\n");
    printf("  🌐 With WiFi & WebSocket Integration\n");
    printf("=========================================================\n");
    printf("Version: 1.0.0 WebSocket Enhanced\n");
    printf("Platform: Raspberry Pi Pico W\n");
    printf("MCU: RP2040 @ 125 MHz\n");
    printf("WiFi: CYW43439 (802.11b/g/n)\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("=========================================================\n");
    printf("\n");
}

/**
 * @brief Setup WiFi connection and WebSocket server
 */
static bool setup_wifi_connection(void)
{
    if (!pico_w_initialized)
    {
        printf("[WIFI] Pico W hardware not initialized\n");
        return false;
    }

    // Initialize WiFi manager
    if (!wifi_manager_init())
    {
        printf("[WIFI] Failed to initialize WiFi manager\n");
        return false;
    }

    // Register simplified WiFi event callback (no actual callback in simplified version)
    wifi_register_event_callback(NULL);

    // Set hostname
    wifi_set_hostname(WIFI_HOSTNAME);

    // Connect to WiFi
    bool wifi_connected = false;
    wifi_connection_attempts = 0;

    if (USE_HARDCODED_WIFI)
    {
        printf("[WIFI] Connecting to %s...\n", WIFI_SSID);
        wifi_connection_attempts++;
        wifi_connected = wifi_connect(WIFI_SSID, WIFI_PASSWORD);

        if (wifi_connected)
        {
            printf("[WIFI] Connection successful!\n");
        }
        else
        {
            printf("[WIFI] Failed to connect to WiFi\n");
            printf("[WIFI] Will continue attempting to connect in background\n");
        }
    }
    else
    {
        configure_wifi_via_uart();
        // Connection will be handled when UART command is received
    }

    wifi_setup_complete = true;
    return true;
}

/**
 * @brief Configure WiFi via UART commands
 */
static void configure_wifi_via_uart(void)
{
    printf("\n");
    printf("=== WiFi Configuration Mode ===\n");
    printf("No hardcoded WiFi credentials configured.\n");
    printf("Send UART commands to configure WiFi:\n");
    printf("  WIFI_CONNECT <SSID> <PASSWORD>\n");
    printf("  Example: WIFI_CONNECT MyNetwork MyPassword123\n");
    printf("  For open networks: WIFI_CONNECT MyNetwork\n");
    printf("  Check status: WIFI_STATUS\n");
    printf("  Disconnect: WIFI_DISCONNECT\n");
    printf("================================\n");
    printf("\n");
}

/**
 * @brief Handle UART WiFi commands
 */
static void handle_uart_wifi_commands(void)
{
    // This function would be called from the UART handler
    // Implementation depends on your UART command parsing system

    // Example implementation (you'd integrate this with your existing UART handler):
    
    if (strncmp(uart_command, "WIFI_CONNECT", 12) == 0)
    {
        char ssid[WIFI_SSID_MAX_LENGTH];
        char password[WIFI_PASSWORD_MAX_LENGTH];

        // Parse command: WIFI_CONNECT <SSID> <PASSWORD>
        int parsed = sscanf(uart_command + 13, "%31s %63s", ssid, password);

        if (parsed >= 1)
        {
            strncpy(wifi_ssid_buffer, ssid, sizeof(wifi_ssid_buffer) - 1);
            if (parsed >= 2)
            {
                strncpy(wifi_password_buffer, password, sizeof(wifi_password_buffer) - 1);
            }
            else
            {
                wifi_password_buffer[0] = '\0'; // Open network
            }

            wifi_configured_via_uart = true;
            wifi_connection_attempts++;

            printf("[WIFI] Attempting to connect to %s...\n", wifi_ssid_buffer);
            if (wifi_connect(wifi_ssid_buffer, wifi_password_buffer))
            {
                printf("[WIFI] Connection initiated\n");
            }
            else
            {
                printf("[WIFI] Connection failed\n");
            }
        }
    }
    else if (strncmp(uart_command, "WIFI_STATUS", 11) == 0)
    {
        if (wifi_is_connected())
        {
            printf("[WIFI] Status: Connected to %s\n", wifi_get_ssid());
            printf("[WIFI] IP Address: %s\n", wifi_get_ip_address());
            printf("[WIFI] RSSI: %d dBm\n", wifi_get_rssi());
        }
        else
        {
            printf("[WIFI] Status: Disconnected\n");
            printf("[WIFI] Attempts: %lu\n", wifi_connection_attempts);
        }
    }
    else if (strncmp(uart_command, "WIFI_DISCONNECT", 15) == 0)
    {
        wifi_disconnect();
        printf("[WIFI] Disconnected\n");
    }
    
}

/**
 * @brief Simplified WiFi event handler
 */
static void wifi_event_handler_simplified(void)
{
    // Simplified event handling - just check if we're connected
    if (wifi_is_connected())
    {
        printf("[WIFI] WiFi connection detected\n");

        // Initialize WebSocket server now that WiFi is connected
        if (!websocket_setup_complete)
        {
            printf("[WEBSOCKET] Starting WebSocket server...\n");
            if (websocket_server_init())
            {
                websocket_setup_complete = true;

                // Register WebSocket callbacks
                websocket_register_command_callback(websocket_command_handler);
                websocket_register_client_callback(websocket_client_handler);

                printf("[WEBSOCKET] WebSocket server started on port %d\n", NET_WEBSOCKET_PORT);
                websocket_send_log("info", "WebSocket", "Server started and ready for connections");
            }
            else
            {
                printf("[WEBSOCKET] Failed to start WebSocket server\n");
            }
        }

        if (websocket_setup_complete)
        {
            websocket_send_log("info", "WiFi", "Successfully connected to network");

            // Send initial system status
            send_system_status_update();
        }
    }
    else
    {
        printf("[WIFI] WiFi disconnected\n");
        if (websocket_setup_complete)
        {
            websocket_send_log("warn", "WiFi", "Disconnected from network");
        }
    }
}
register_command_callback(websocket_command_handler);
websocket_register_client_callback(websocket_client_handler);

printf("[WEBSOCKET] WebSocket server started on port %d\n", NET_WEBSOCKET_PORT);
websocket_send_log("info", "WebSocket", "Server started and ready for connections");
}
else
{
    printf("[WEBSOCKET] Failed to start WebSocket server\n");
}
}

if (websocket_setup_complete)
{
    websocket_send_log("info", "WiFi", "Successfully connected to network");

    // Send initial system status
    send_system_status_update();
}
break;

case WIFI_EVENT_DISCONNECTED:
printf("[WIFI] Disconnected from WiFi\n");
if (websocket_setup_complete)
{
    websocket_send_log("warn", "WiFi", "Disconnected from network");
}

// Attempt reconnection if we have credentials
if (USE_HARDCODED_WIFI || wifi_configured_via_uart)
{
    printf("[WIFI] Attempting reconnection in %d seconds...\n", WIFI_RECONNECT_DELAY_MS / 1000);
    // Note: Actual reconnection logic would be handled by the WiFi manager
}
break;

case WIFI_EVENT_CONNECTION_FAILED:
wifi_connection_attempts++;
printf("[WIFI] Connection failed (attempt %lu)\n", wifi_connection_attempts);
if (websocket_setup_complete)
{
    websocket_send_log("error", "WiFi", "Connection failed");
}

if (wifi_connection_attempts < WIFI_MAX_RETRY_COUNT)
{
    printf("[WIFI] Will retry connection...\n");
}
else
{
    printf("[WIFI] Maximum retry attempts reached\n");
}
break;

default:
break;
}
}

/**
 * @brief WebSocket command handler callback
 */
static bool websocket_command_handler(const char *command, const char *params, int client_id)
{
    printf("[WEBSOCKET] Command from client %d: %s %s\n", client_id, command, params ? params : "");

    // Handle specific commands
    if (strcmp(command, "GET_STATUS") == 0)
    {
        send_system_status_update();
        return true;
    }
    else if (strcmp(command, "GET_CHANNELS") == 0)
    {
        send_channel_updates();
        return true;
    }
    else if (strcmp(command, "WIFI_STATUS") == 0)
    {
        // Send WiFi status
        char wifi_status[256];
        if (wifi_is_connected())
        {
            snprintf(wifi_status, sizeof(wifi_status),
                     "Connected to %s, IP: %s, RSSI: %d dBm",
                     wifi_get_ssid(), wifi_get_ip_address(), wifi_get_rssi());
        }
        else
        {
            snprintf(wifi_status, sizeof(wifi_status), "Disconnected");
        }
        websocket_send_log("info", "WiFi", wifi_status);
        return true;
    }

    // Log command execution
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Command executed: %s", command);
    websocket_send_log("info", "Command", log_msg);

    return true;
}

/**
 * @brief WebSocket client connection handler callback
 */
static void websocket_client_handler(int client_id, bool connected, const char *client_ip)
{
    if (connected)
    {
        printf("[WEBSOCKET] Client %d connected from %s\n", client_id, client_ip ? client_ip : "unknown");

        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Client connected from %s", client_ip ? client_ip : "unknown");
        websocket_send_log("info", "WebSocket", log_msg);

        // Send welcome message and initial status
        send_system_status_update();
    }
    else
    {
        printf("[WEBSOCKET] Client %d disconnected\n", client_id);
        websocket_send_log("info", "WebSocket", "Client disconnected");
    }
}

/**
 * @brief Update WiFi LED status based on connection state
 */
static void update_wifi_led_status(void)
{
    if (!pico_w_initialized)
        return;

    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Use Pico SDK time

    if (wifi_is_connected())
    {
        // Slow blink when connected (every 2 seconds)
        if (current_time - last_wifi_led_update >= WIFI_LED_BLINK_CONNECTED_MS)
        {
            wifi_led_state = !wifi_led_state;
            wifi_set_led(wifi_led_state);
            last_wifi_led_update = current_time;
        }
    }
    else if (wifi_connection_attempts > 0)
    {
        // Fast blink when trying to connect (every 200ms)
        if (current_time - last_wifi_led_update >= WIFI_LED_BLINK_CONNECTING_MS)
        {
            wifi_led_state = !wifi_led_state;
            wifi_set_led(wifi_led_state);
            last_wifi_led_update = current_time;
        }
    }
    else
    {
        // Off when disconnected and not trying to connect
        wifi_set_led(false);
        wifi_led_state = false;
    }
}

/**
 * @brief Send system status update via WebSocket
 */
static void send_system_status_update(void)
{
    if (!websocket_setup_complete)
        return;

    // Create system status message (simplified - remove missing functions)
    char status_msg[512];
    snprintf(status_msg, sizeof(status_msg),
             "System Status: Online | WiFi: %s | Uptime: %lu ms",
             wifi_is_connected() ? "Connected" : "Disconnected",
             to_ms_since_boot(get_absolute_time()) // Use Pico SDK time function
    );

    websocket_send_log("info", "Status", status_msg);
}

/**
 * @brief Setup emergency stop handling
 */
static void setup_emergency_stop(void)
{
    printf("[MAIN] Setting up emergency stop handling...\n");

    // Register emergency stop callback with input handler
    register_emergency_stop_callback(system_emergency_stop_handler);

    // Register emergency stop callback with safety monitor
    register_safety_emergency_callback(system_emergency_stop_handler);

    printf("[MAIN] Emergency stop handling configured\n");
}

/**
 * @brief Emergency stop handler - called when emergency stop is triggered
 */
static void system_emergency_stop_handler(void)
{
    printf("\n");
    printf("!!! EMERGENCY STOP TRIGGERED !!!\n");
    printf("Initiating immediate system shutdown...\n");

    // Send emergency notification via WebSocket
    if (websocket_setup_complete)
    {
        websocket_send_log("error", "Emergency", "EMERGENCY STOP ACTIVATED - All channels disabled");
    }

    // Perform emergency shutdown
    emergency_shutdown("Emergency stop button pressed or safety violation");

    // Request main loop to stop
    request_system_stop();
}

/**
 * @brief Send periodic channel data updates via WebSocket
 */
static void send_channel_updates(void)
{
    if (!websocket_setup_complete)
    {
        return;
    }

    // Read and send channel data for each diagnostic channel
    for (int channel = 1; channel <= NUM_DIAGNOSTIC_CHANNELS; channel++)
    {
        // Read voltage and current for this channel
        uint16_t adc_value;
        float voltage = 0.0f;
        float current = 0.0f;
        bool channel_enabled = false;

        // Check if channel is enabled (simplified - use direct GPIO read)
        switch (channel)
        {
        case 1:
        {
            gpio_state_t state;
            if (hal_gpio_read(DIAG_CH1_ENABLE_PIN, &state) == HAL_OK)
            {
                channel_enabled = (state == GPIO_STATE_HIGH);
            }
        }
        break;
        case 2:
        {
            gpio_state_t state;
            if (hal_gpio_read(DIAG_CH2_ENABLE_PIN, &state) == HAL_OK)
            {
                channel_enabled = (state == GPIO_STATE_HIGH);
            }
        }
        break;
        case 3:
        {
            gpio_state_t state;
            if (hal_gpio_read(DIAG_CH3_ENABLE_PIN, &state) == HAL_OK)
            {
                channel_enabled = (state == GPIO_STATE_HIGH);
            }
        }
        break;
        case 4:
        {
            gpio_state_t state;
            if (hal_gpio_read(DIAG_CH4_ENABLE_PIN, &state) == HAL_OK)
            {
                channel_enabled = (state == GPIO_STATE_HIGH);
            }
        }
        break;
        }

        if (channel_enabled)
        {
            // Read voltage (map channels to appropriate ADC inputs)
            if (channel <= 2)
            {
                if (hal_adc_read(channel - 1, &adc_value) == HAL_OK)
                {
                    voltage = ADC_TO_VOLTAGE(adc_value) * (CHANNEL_VOLTAGE_RANGE / 3.3f);
                }
            }

            // Read current (simplified - you'd implement proper current sensing)
            if (channel == 3)
            {
                if (hal_adc_read(ADC_CH3_CURRENT, &adc_value) == HAL_OK)
                {
                    current = ADC_TO_VOLTAGE(adc_value) * (CHANNEL_CURRENT_RANGE / 3.3f);
                }
            }
        }

        // Send channel data via WebSocket (use the simplified 3-parameter version)
        websocket_send_channel_data(channel, voltage, current);
    }
}

/**
 * @brief Update web integration components
 */
static void web_integration_update(void)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time()); // Use Pico SDK time

    // Update WiFi manager
    if (wifi_setup_complete)
    {
        wifi_manager_update();

        // Check WiFi status periodically and handle events
        static bool last_wifi_status = false;
        bool current_wifi_status = wifi_is_connected();
        if (current_wifi_status != last_wifi_status)
        {
            wifi_event_handler_simplified();
            last_wifi_status = current_wifi_status;
        }
    }

    // Update WiFi LED status
    update_wifi_led_status();

    // Update WebSocket server
    if (websocket_setup_complete)
    {
        websocket_server_update();
    }

    // Send periodic channel updates
    if (current_time - last_channel_update >= 1000) // Every 1 second
    {
        send_channel_updates();
        last_channel_update = current_time;
    }

    // Send periodic status updates
    if (current_time - last_status_update >= STATUS_UPDATE_INTERVAL_MS) // Every 5 seconds
    {
        send_system_status_update();
        last_status_update = current_time;
    }

    // General web integration updates
    if (current_time - last_web_update >= 100) // Every 100ms
    {
        // Handle any UART WiFi commands
        handle_uart_wifi_commands();

        // Check for WiFi reconnection if needed
        if (!wifi_is_connected() && (USE_HARDCODED_WIFI || wifi_configured_via_uart))
        {
            // WiFi manager should handle automatic reconnection
        }

        last_web_update = current_time;
    }
}

/**
 * @brief Cleanup and exit the application
 */
static void cleanup_and_exit(void)
{
    printf("\n[MAIN] Starting system cleanup...\n");

    // Send shutdown notification
    if (websocket_setup_complete)
    {
        websocket_send_log("info", "System", "System shutdown initiated");
        sleep_ms(100); // Give time for message to send
    }

    // Stop WebSocket server
    if (websocket_setup_complete)
    {
        websocket_server_stop();
        websocket_setup_complete = false;
    }

    // Disconnect WiFi
    if (wifi_setup_complete)
    {
        wifi_disconnect();
        wifi_manager_deinit();
        wifi_setup_complete = false;
    }

    // Deinitialize CYW43 WiFi chip
    if (pico_w_initialized)
    {
        cyw43_arch_deinit();
        pico_w_initialized = false;
    }

    // Disable input processing
    set_input_processing_enabled(false);

    // Clear any pending input events
    clear_input_events();

    // Deinitialize all systems
    hal_status_t deinit_status = system_deinit();
    if (deinit_status != HAL_OK)
    {
        printf("[MAIN] WARNING: System deinitialization had errors (status: %d)\n", deinit_status);
    }

    printf("[MAIN] System cleanup complete\n");
    printf("[MAIN] Application exiting\n");
    printf("\n");
    printf("=========================================================\n");
    printf("  Multi-Channel Diagnostic Test Rig - Shutdown Complete\n");
    printf("=========================================================\n");
}

// =============================================================================
// ENHANCED MAIN LOOP INTEGRATION
// =============================================================================

// Override the main loop to include web integration
// This can be called from your existing run_main_loop() function
void integrate_web_updates_in_main_loop(void)
{
    // Call this function from within your main loop
    web_integration_update();
}

// Example of how to modify your existing main loop:
/*
void run_main_loop(void)
{
    printf("[LOOP] Starting main application loop...\n");

    uint32_t loop_counter = 0;
    bool system_stop_requested = false;

    while (!system_stop_requested)
    {
        loop_counter++;

        // Existing loop functionality
        handle_user_input();
        check_system_safety();

        // Add web integration updates
        integrate_web_updates_in_main_loop();

        // Rest of your existing loop...
        if (loop_counter % 1000 == 0)
        {
            hal_gpio_toggle(LED_STATUS_PIN);
            printf("[LOOP] Heartbeat: %lu loops\n", loop_counter);
        }

        system_stop_requested = is_system_stop_requested();
        hal_delay_ms(MAIN_LOOP_DELAY_MS);
    }

    printf("[LOOP] Main loop exiting after %lu iterations\n", loop_counter);
}
*/