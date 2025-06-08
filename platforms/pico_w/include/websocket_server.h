/**
 * @file websocket_server.h
 * @brief Simplified WebSocket server header for Raspberry Pi Pico W
 */

#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // TYPE DEFINITIONS
    // =============================================================================

    /**
     * @brief WebSocket command callback function type
     * @param command The command string
     * @param params The parameters string (can be NULL)
     * @param client_id The client ID that sent the command
     * @return true if command was handled, false otherwise
     */
    typedef bool (*websocket_command_callback_t)(const char *command, const char *params, int client_id);

    /**
     * @brief WebSocket client connection callback function type
     * @param client_id The client ID
     * @param connected true if client connected, false if disconnected
     * @param client_ip The client IP address string
     */
    typedef void (*websocket_client_callback_t)(int client_id, bool connected, const char *client_ip);

    // =============================================================================
    // PUBLIC FUNCTION DECLARATIONS
    // =============================================================================

    /**
     * @brief Initialize the WebSocket server
     * @return true if initialization was successful, false otherwise
     */
    bool websocket_server_init(void);

    /**
     * @brief Update the WebSocket server (call this periodically)
     */
    void websocket_server_update(void);

    /**
     * @brief Stop the WebSocket server and clean up resources
     */
    void websocket_server_stop(void);

    /**
     * @brief Register a callback for WebSocket commands
     * @param callback The callback function to register
     */
    void websocket_register_command_callback(websocket_command_callback_t callback);

    /**
     * @brief Register a callback for client connection events
     * @param callback The callback function to register
     */
    void websocket_register_client_callback(websocket_client_callback_t callback);

    /**
     * @brief Send a log message to all connected WebSocket clients
     * @param level The log level (e.g., "info", "warn", "error")
     * @param category The log category (e.g., "WiFi", "System")
     * @param message The log message
     */
    void websocket_send_log(const char *level, const char *category, const char *message);

    /**
     * @brief Send channel data to all connected WebSocket clients
     * @param channel The channel number (1-4)
     * @param voltage The voltage reading
     * @param current The current reading
     */
    void websocket_send_channel_data(int channel, float voltage, float current);

#ifdef __cplusplus
}
#endif

#endif // WEBSOCKET_SERVER_H