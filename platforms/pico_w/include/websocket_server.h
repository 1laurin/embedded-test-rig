/**
 * @file websocket_server.h
 * @brief WebSocket server interface for Pico W diagnostic test rig
 *
 * This file provides the interface for the WebSocket server that enables
 * real-time communication between the Pico W and web-based diagnostic interface.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
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
    // CONFIGURATION CONSTANTS
    // =============================================================================

#define WEBSOCKET_PORT 8080
#define WEBSOCKET_MAX_CLIENTS 4
#define WEBSOCKET_BUFFER_SIZE 1024
#define WEBSOCKET_PING_INTERVAL_MS 30000
#define WEBSOCKET_STATUS_UPDATE_INTERVAL_MS 2000

    // =============================================================================
    // MESSAGE TYPES
    // =============================================================================

    typedef enum
    {
        WS_MSG_TYPE_STATUS = 0,
        WS_MSG_TYPE_CHANNEL_DATA,
        WS_MSG_TYPE_SYSTEM_INFO,
        WS_MSG_TYPE_LOG,
        WS_MSG_TYPE_ERROR,
        WS_MSG_TYPE_COMMAND_ACK
    } websocket_message_type_t;

    typedef enum
    {
        WS_LOG_LEVEL_DEBUG = 0,
        WS_LOG_LEVEL_INFO,
        WS_LOG_LEVEL_WARN,
        WS_LOG_LEVEL_ERROR
    } websocket_log_level_t;

    // =============================================================================
    // CLIENT INFORMATION STRUCTURE
    // =============================================================================

    typedef struct
    {
        char ip_address[16];
        uint32_t connect_time;
        uint32_t last_activity;
        uint32_t messages_sent;
        uint32_t messages_received;
        bool authenticated;
    } websocket_client_info_t;

    // =============================================================================
    // PUBLIC FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize the WebSocket server
     * @return true on success, false on failure
     */
    bool websocket_server_init(void);

    /**
     * @brief Update the WebSocket server (handle connections and messages)
     * This should be called regularly from the main loop
     */
    void websocket_server_update(void);

    /**
     * @brief Stop the WebSocket server and cleanup resources
     */
    void websocket_server_stop(void);

    /**
     * @brief Check if the WebSocket server is running
     * @return true if running, false otherwise
     */
    bool websocket_server_is_running(void);

    /**
     * @brief Get the number of connected clients
     * @return Number of active WebSocket connections
     */
    int websocket_server_get_client_count(void);

    /**
     * @brief Get information about connected clients
     * @param clients Array to store client information
     * @param max_clients Maximum number of clients to return
     * @return Actual number of clients returned
     */
    int websocket_server_get_clients(websocket_client_info_t *clients, int max_clients);

    // =============================================================================
    // MESSAGE SENDING FUNCTIONS
    // =============================================================================

    /**
     * @brief Send a log message to all connected clients
     * @param level Log level (debug, info, warn, error)
     * @param source Source of the log message
     * @param message Log message content
     */
    void websocket_send_log(const char *level, const char *source, const char *message);

    /**
     * @brief Send channel data update to all connected clients
     * @param channel Channel number (1-4)
     * @param voltage Channel voltage reading
     * @param current Channel current reading
     */
    void websocket_send_channel_data(int channel, float voltage, float current);

    /**
     * @brief Send system status update to all connected clients
     * @param temperature System temperature
     * @param uptime_seconds System uptime in seconds
     * @param loop_count Main loop iteration count
     * @param free_memory Available memory in KB
     */
    void websocket_send_system_status(float temperature, uint32_t uptime_seconds,
                                      uint32_t loop_count, uint32_t free_memory);

    /**
     * @brief Send channel states to all connected clients
     * @param channel_states Array of 4 boolean values for channel states
     */
    void websocket_send_channel_states(const bool *channel_states);

    /**
     * @brief Send error message to all connected clients
     * @param error_code Error code
     * @param error_message Error description
     */
    void websocket_send_error(int error_code, const char *error_message);

    /**
     * @brief Send command acknowledgment to specific client
     * @param client_id Client identifier
     * @param command Command that was executed
     * @param success Whether command succeeded
     * @param response Response message
     */
    void websocket_send_command_ack(int client_id, const char *command, bool success, const char *response);

    /**
     * @brief Broadcast a raw JSON message to all connected clients
     * @param json_message JSON formatted message string
     */
    void websocket_broadcast_json(const char *json_message);

    /**
     * @brief Send a raw JSON message to a specific client
     * @param client_id Client identifier
     * @param json_message JSON formatted message string
     */
    void websocket_send_json(int client_id, const char *json_message);

    // =============================================================================
    // CALLBACK REGISTRATION
    // =============================================================================

    /**
     * @brief Callback function type for handling incoming commands
     * @param command Command string
     * @param params JSON parameters string
     * @param client_id Client that sent the command
     * @return true if command was handled successfully
     */
    typedef bool (*websocket_command_callback_t)(const char *command, const char *params, int client_id);

    /**
     * @brief Callback function type for client connection events
     * @param client_id Client identifier
     * @param connected true for connect, false for disconnect
     * @param client_ip Client IP address
     */
    typedef void (*websocket_client_callback_t)(int client_id, bool connected, const char *client_ip);

    /**
     * @brief Register a callback for handling incoming commands
     * @param callback Function to call when commands are received
     */
    void websocket_register_command_callback(websocket_command_callback_t callback);

    /**
     * @brief Register a callback for client connection events
     * @param callback Function to call when clients connect/disconnect
     */
    void websocket_register_client_callback(websocket_client_callback_t callback);

    // =============================================================================
    // UTILITY FUNCTIONS
    // =============================================================================

    /**
     * @brief Get server statistics
     * @param total_connections Total connections since startup
     * @param active_connections Currently active connections
     * @param messages_sent Total messages sent
     * @param messages_received Total messages received
     * @param uptime_ms Server uptime in milliseconds
     */
    void websocket_get_server_stats(uint32_t *total_connections, uint32_t *active_connections,
                                    uint32_t *messages_sent, uint32_t *messages_received,
                                    uint32_t *uptime_ms);

    /**
     * @brief Send a ping to all connected clients
     * This can be used to test connectivity and keep connections alive
     */
    void websocket_ping_all_clients(void);

    /**
     * @brief Set the status update interval
     * @param interval_ms Interval in milliseconds between automatic status updates
     */
    void websocket_set_status_update_interval(uint32_t interval_ms);

    /**
     * @brief Enable or disable automatic status broadcasting
     * @param enabled true to enable, false to disable
     */
    void websocket_set_auto_status_enabled(bool enabled);

#ifdef __cplusplus
}
#endif

#endif // WEBSOCKET_SERVER_H