/**
 * @file main.cpp
 * @brief Main application with WebSocket integration for Raspberry Pi Pico W
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
#include <stdio.h>

// =============================================================================
// WIFI CREDENTIALS (Change these for your network)
// =============================================================================

#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"

// Set to true to use the above credentials, false to configure via UART
#define USE_HARDCODED_WIFI false

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void print_startup_banner(void);
static void setup_emergency_stop(void);
static void system_emergency_stop_handler(void);
static void cleanup_and_exit(void);
static bool setup_wifi_connection(void);
static void wifi_event_handler(wifi_event_t event, const wifi_config_t *config);
static bool websocket_command_handler(const char *command, const char *params, int client_id);
static void websocket_client_handler(int client_id, bool connected, const char *client_ip);
static void configure_wifi_via_uart(void);
static void send_channel_updates(void);
static void web_integration_update(void);

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool wifi_setup_complete = false;
static bool websocket_setup_complete = false;
static uint32_t last_channel_update = 0;
static uint32_t last_web_update = 0;

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main()
{
    // Print startup banner
    print_startup_banner();

    // Initialize the entire system
    printf("[MAIN] Initializing core system...\n");
    hal_status_t init_status = system_init();
    if (init_status != HAL_OK)
    {
        printf("CRITICAL ERROR: System initialization failed (status: %d)\n", init_status);
        printf("System cannot continue. Please check hardware connections.\n");
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
        websocket_send_log("warn", "Network", "WiFi connection failed - running in offline mode");
    }

    // Run HAL feature demonstration
    printf("[MAIN] Running HAL demonstration...\n");
    run_hal_demo();
    websocket_send_log("info", "HAL", "HAL demonstration completed");

    // Test all HAL subsystems
    printf("[MAIN] Testing HAL subsystems...\n");
    if (!test_hal_subsystems())
    {
        printf("WARNING: Some HAL subsystem tests failed\n");
        websocket_send_log("warn", "HAL", "Some HAL subsystem tests failed");
    }
    else
    {
        websocket_send_log("info", "HAL", "All HAL subsystem tests passed");
    }

    printf("\n");
    printf("=======================================================\n");
    printf(" System Ready - Starting Main Application Loop\n");
    printf("=======================================================\n");
    if (wifi_is_connected())
    {
        printf("📡 WiFi Connected: %s\n", wifi_get_ip_address());
        printf("🌐 WebSocket Server: http://%s:8080\n", wifi_get_ip_address());
        printf("🖥️  Web Interface: http://%s/static/\n", wifi_get_ip_address());
    }
    printf("🎛️  Press user button to toggle diagnostic channels\n");
    printf("📡 Send UART commands for remote control\n");
    printf("🛡️  System performing automatic safety monitoring\n");
    printf("\n");

    websocket_send_log("info", "System", "Multi-Channel Diagnostic Test Rig online and ready");

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
 * @brief Print the startup banner
 */
static void print_startup_banner(void)
{
    printf("\n");
    printf("=========================================================\n");
    printf("  Multi-Channel Diagnostic Test Rig - Pico W Edition\n");
    printf("  🌐 With WiFi & WebSocket Integration\n");
    printf("=========================================================\n");
    printf("Version: 1.0.0 WebSocket\n");
    printf("Platform: Raspberry Pi Pico W\n");
    printf("MCU: RP2040 @ 125 MHz\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("=========================================================\n");
    printf("\n");
}

/**
 * @brief Setup WiFi connection and WebSocket server
 */
static bool setup_wifi_connection(void)
{
    // Initialize WiFi manager
    if (!wifi_manager_init())
    {
        printf("[WIFI] Failed to initialize WiFi manager\n");
        return false;
    }

    // Register WiFi event callback
    wifi_register_event_callback(wifi_event_handler);

    // Set hostname
    wifi_set_hostname(WIFI_HOSTNAME);

    // Connect to WiFi
    bool wifi_connected = false;

    if (USE_HARDCODED_WIFI)
    {
        printf("[WIFI] Connecting to %s...\n", WIFI_SSID);
        wifi_connected = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
    }
    else
    {
        configure_wifi_via_uart();
        // After configuration, connection will be handled by the event callback
        wifi_connected = true; // Assume configuration was successful
    }

    if (!wifi_connected && USE_HARDCODED_WIFI)
    {
        printf("[WIFI] Failed to connect to WiFi\n");
        return false;
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
    printf("=== WiFi Configuration ===\n");
    printf("No hardcoded WiFi credentials found.\n");
    printf("Send UART commands to configure WiFi:\n");
    printf("  WIFI_CONNECT <SSID> <PASSWORD>\n");
    printf("  Example: WIFI_CONNECT MyNetwork MyPassword\n");
    printf("  For open networks: WIFI_CONNECT MyNetwork\n");
    printf("===============================\n");
    printf("\n");
}

/**
 * @brief WiFi event handler callback
 */
static void wifi_event_handler(wifi_event_t event, const wifi_config_t *config)
{
    switch (event)
    {
    case WIFI_EVENT_CONNECTING:
        printf("[WIFI] Connecting to %s...\n", config->ssid);
        websocket_send_log("info", "WiFi", "Connecting to network...");
        break;

    case WIFI_EVENT_CONNECTED:
        printf("[WIFI] Connected to %s\n", config->ssid);
        printf("[WIFI] IP Address: %s\n", wifi_get_ip_address());

        // Initialize WebSocket server now that WiFi is connected
        if (!websocket_setup_complete)
        {
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
                websocket_send_log("error", "WebSocket", "Failed to start server");
            }
        }

        websocket_send_log("info", "WiFi", "Successfully connected to network");
        break;

    case WIFI_EVENT_DISCONNECTED:
        printf("[WIFI] Disconnected from WiFi\n");
        websocket_send_log("warn", "WiFi", "Disconnected from network");
        break;

    case WIFI_EVENT_CONNECTION_FAILED:
        printf("[WIFI] Connection failed\n");
        websocket_send_log("error", "WiFi", "Connection failed");
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

    // Commands are already handled in the WebSocket server implementation
    // This callback is for logging and additional processing if needed

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
        printf("[WEBSOCKET] Client %d connected from %s\n", client_id, client_ip);

        char log_msg[64];
        snprintf(log_msg, sizeof(log_msg), "Client connected from %s", client_ip);
        websocket_send_log("info", "WebSocket", log_msg);
    }
    else
    {
        printf("[WEBSOCKET] Client %d disconnected\n", client_id);
        websocket_send_log("info", "WebSocket", "Client disconnected");
    }
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
    websocket_send_log("error", "Emergency", "EMERGENCY STOP ACTIVATED - All channels disabled");

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
    for (int channel = 1; channel <= 4; channel++)
    {
        // Read voltage and current for this channel
        uint16_t adc_value;
        float voltage = 0.0f;
        float current = 0.0f;

        // Read voltage (simplified - you'd use appropriate ADC channels)
        if (hal_adc_read(channel - 1, &adc_value) == HAL_OK)
        {
            voltage = (float)adc_value * 3.3f / 4096.0f * 10.0f; // Scale for actual voltage range
        }

        // Read current (you'd implement current sensing)
        current = voltage * 0.1f; // Simplified current calculation

        // Send channel data via WebSocket
        websocket_send_channel_data(channel, voltage, current);
    }
}

/**
 * @brief Update web integration components
 */
static void web_integration_update(void)
{
    uint32_t current_time = hal_get_tick_ms();

    // Update WiFi manager
    if (wifi_setup_complete)
    {
        wifi_manager_update();
    }

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

    // General web integration updates
    if (current_time - last_web_update >= 100) // Every 100ms
    {
        // Add any other periodic web-related tasks here
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
    websocket_send_log("info", "System", "System shutdown initiated");

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
// ENHANCED MAIN LOOP (Modified from system_loop.cpp)
// =============================================================================

// We need to override the main loop to include web integration updates
// This replaces the run_main_loop() call with an enhanced version

#ifdef ENABLE_WEB_INTEGRATION_IN_MAIN_LOOP

// This is an alternative main loop that includes web integration
// You can enable this by defining ENABLE_WEB_INTEGRATION_IN_MAIN_LOOP
void run_enhanced_main_loop(void)
{
    printf("[LOOP] Starting enhanced main application loop with web integration...\n");

    uint32_t loop_counter = 0;
    bool system_stop_requested = false;

    while (!system_stop_requested)
    {
        loop_counter++;

        // Handle user input
        handle_user_input();

        // Perform safety checks
        check_system_safety();

        // Update web integration (WiFi + WebSocket)
        web_integration_update();

        // Heartbeat task (blink LED)
        if (loop_counter % 1000 == 0)
        {
            hal_gpio_toggle(LED_STATUS_PIN);
            printf("[LOOP] Heartbeat: %lu loops\n", loop_counter);

            // Send heartbeat via WebSocket
            char heartbeat_msg[64];
            snprintf(heartbeat_msg, sizeof(heartbeat_msg), "Heartbeat - Loop count: %lu", loop_counter);
            websocket_send_log("debug", "Heartbeat", heartbeat_msg);
        }

        // Test diagnostic channels periodically
        if (loop_counter % 5000 == 0)
        {
            test_diagnostic_channels();
        }

        // Check if stop was requested
        system_stop_requested = is_system_stop_requested();

        hal_delay_ms(MAIN_LOOP_DELAY_MS);
    }

    printf("[LOOP] Enhanced main loop exiting after %lu iterations\n", loop_counter);
}

// Replace the main loop call in main() with this enhanced version
// run_enhanced_main_loop();

#endif // ENABLE_WEB_INTEGRATION_IN_MAIN_LOOP