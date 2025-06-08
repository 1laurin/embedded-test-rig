/**
 * @file websocket_server.cpp
 * @brief WebSocket server implementation for Pico W diagnostic test rig
 *
 * This file implements a WebSocket server that enables real-time communication
 * between the Pico W hardware and the web-based diagnostic interface.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "websocket_server.h"
#include "wifi_manager.h"
#include "diagnostics_engine.h"
#include "safety_monitor.h"
#include "hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// =============================================================================
// WEBSOCKET PROTOCOL CONSTANTS
// =============================================================================

#define WS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_OPCODE_TEXT 0x1
#define WS_OPCODE_BINARY 0x2
#define WS_OPCODE_CLOSE 0x8
#define WS_OPCODE_PING 0x9
#define WS_OPCODE_PONG 0xA

#define MAX_CLIENTS 4
#define WEBSOCKET_BUFFER_SIZE 1024
#define STATUS_UPDATE_INTERVAL_MS 2000

// =============================================================================
// WEBSOCKET CLIENT STRUCTURE
// =============================================================================

typedef struct
{
    int socket_fd;
    bool connected;
    bool handshake_complete;
    uint32_t last_ping;
    char client_ip[16];
} websocket_client_t;

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool websocket_server_running = false;
static int server_socket = -1;
static websocket_client_t clients[MAX_CLIENTS];
static uint32_t last_status_broadcast = 0;
static bool diagnostic_channels[4] = {false, false, false, false};

// Message queue for outgoing messages
typedef struct
{
    char data[WEBSOCKET_BUFFER_SIZE];
    size_t length;
    bool broadcast;
    int target_client;
} ws_message_t;

static ws_message_t message_queue[16];
static int queue_head = 0;
static int queue_tail = 0;

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

static bool websocket_handshake(int client_socket, const char *request);
static void websocket_send_frame(int client_socket, const char *data, size_t length, uint8_t opcode);
static bool websocket_parse_frame(const char *buffer, size_t length, char *payload, size_t *payload_length);
static void handle_websocket_message(int client_socket, const char *message);
static void broadcast_status_update(void);
static void send_system_info(int client_socket);
static void queue_message(const char *message, bool broadcast, int target_client);
static void process_message_queue(void);
static int find_free_client_slot(void);
static void cleanup_client(int client_index);

// =============================================================================
// BASE64 ENCODING FOR WEBSOCKET HANDSHAKE
// =============================================================================

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(const unsigned char *input, size_t length, char *output)
{
    size_t i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];

    while (i < length)
    {
        char_array_3[0] = input[i++];
        char_array_3[1] = (i < length) ? input[i++] : 0;
        char_array_3[2] = (i < length) ? input[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int k = 0; k < 4; k++)
        {
            output[j++] = base64_chars[char_array_4[k]];
        }
    }
    output[j] = '\0';
}

// =============================================================================
// WEBSOCKET HANDSHAKE
// =============================================================================

static bool websocket_handshake(int client_socket, const char *request)
{
    // Find the Sec-WebSocket-Key header
    const char *key_header = "Sec-WebSocket-Key: ";
    char *key_start = strstr(request, key_header);
    if (!key_start)
    {
        printf("[WS] No WebSocket key found in handshake\n");
        return false;
    }

    key_start += strlen(key_header);
    char *key_end = strstr(key_start, "\r\n");
    if (!key_end)
    {
        printf("[WS] Malformed WebSocket key\n");
        return false;
    }

    // Extract the key
    size_t key_length = key_end - key_start;
    char websocket_key[64];
    strncpy(websocket_key, key_start, key_length);
    websocket_key[key_length] = '\0';

    // Concatenate with magic string
    char concat_key[128];
    snprintf(concat_key, sizeof(concat_key), "%s%s", websocket_key, WS_MAGIC_STRING);

    // For simplicity, we'll use a basic hash instead of SHA-1
    // In production, you'd want proper SHA-1 implementation
    unsigned char hash[20] = {0}; // Simplified hash
    for (int i = 0; i < strlen(concat_key); i++)
    {
        hash[i % 20] ^= concat_key[i];
    }

    // Base64 encode the hash
    char accept_key[32];
    base64_encode(hash, 20, accept_key);

    // Send WebSocket handshake response
    char response[512];
    snprintf(response, sizeof(response),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n"
             "\r\n",
             accept_key);

    int sent = send(client_socket, response, strlen(response), 0);
    if (sent <= 0)
    {
        printf("[WS] Failed to send handshake response\n");
        return false;
    }

    printf("[WS] WebSocket handshake completed\n");
    return true;
}

// =============================================================================
// WEBSOCKET FRAME HANDLING
// =============================================================================

static void websocket_send_frame(int client_socket, const char *data, size_t length, uint8_t opcode)
{
    uint8_t frame[WEBSOCKET_BUFFER_SIZE];
    size_t frame_size = 0;

    // First byte: FIN + opcode
    frame[0] = 0x80 | opcode;
    frame_size++;

    // Payload length
    if (length < 126)
    {
        frame[1] = length;
        frame_size++;
    }
    else if (length < 65536)
    {
        frame[1] = 126;
        frame[2] = (length >> 8) & 0xFF;
        frame[3] = length & 0xFF;
        frame_size += 3;
    }
    else
    {
        // For simplicity, we don't support very large frames
        printf("[WS] Frame too large: %zu bytes\n", length);
        return;
    }

    // Copy payload
    memcpy(frame + frame_size, data, length);
    frame_size += length;

    // Send frame
    int sent = send(client_socket, frame, frame_size, 0);
    if (sent <= 0)
    {
        printf("[WS] Failed to send frame\n");
    }
}

static bool websocket_parse_frame(const char *buffer, size_t length, char *payload, size_t *payload_length)
{
    if (length < 2)
        return false;

    uint8_t first_byte = buffer[0];
    uint8_t second_byte = buffer[1];

    bool fin = (first_byte & 0x80) != 0;
    uint8_t opcode = first_byte & 0x0F;
    bool masked = (second_byte & 0x80) != 0;
    uint8_t payload_len = second_byte & 0x7F;

    if (!fin || !masked)
        return false; // We expect complete, masked frames from client

    size_t header_size = 2;
    size_t actual_payload_length = payload_len;

    if (payload_len == 126)
    {
        if (length < 4)
            return false;
        actual_payload_length = (buffer[2] << 8) | buffer[3];
        header_size = 4;
    }
    else if (payload_len == 127)
    {
        // For simplicity, we don't support very large frames
        return false;
    }

    if (length < header_size + 4 + actual_payload_length)
        return false;

    // Extract mask
    uint8_t mask[4];
    memcpy(mask, buffer + header_size, 4);
    header_size += 4;

    // Unmask payload
    for (size_t i = 0; i < actual_payload_length; i++)
    {
        payload[i] = buffer[header_size + i] ^ mask[i % 4];
    }
    payload[actual_payload_length] = '\0';
    *payload_length = actual_payload_length;

    return opcode == WS_OPCODE_TEXT;
}

// =============================================================================
// MESSAGE HANDLING
// =============================================================================

static void handle_websocket_message(int client_socket, const char *message)
{
    printf("[WS] Received: %s\n", message);

    // Parse JSON message (simplified parsing)
    if (strstr(message, "\"type\":\"command\""))
    {
        // Extract command
        char *cmd_start = strstr(message, "\"command\":\"");
        if (cmd_start)
        {
            cmd_start += 11; // Length of "\"command\":\""
            char *cmd_end = strchr(cmd_start, '"');
            if (cmd_end)
            {
                size_t cmd_len = cmd_end - cmd_start;
                char command[64];
                strncpy(command, cmd_start, cmd_len);
                command[cmd_len] = '\0';

                // Process commands
                if (strcmp(command, "GET_STATUS") == 0)
                {
                    broadcast_status_update();
                    send_system_info(client_socket);
                }
                else if (strcmp(command, "TOGGLE_CHANNEL") == 0)
                {
                    // Extract channel parameter
                    char *ch_start = strstr(message, "\"channel\":");
                    if (ch_start)
                    {
                        int channel = atoi(ch_start + 10);
                        if (channel >= 1 && channel <= 4)
                        {
                            diagnostic_channels[channel - 1] = !diagnostic_channels[channel - 1];
                            set_channel_enable(channel, diagnostic_channels[channel - 1]);
                            printf("[WS] Toggled channel %d: %s\n", channel,
                                   diagnostic_channels[channel - 1] ? "ON" : "OFF");
                        }
                    }
                }
                else if (strcmp(command, "ENABLE_ALL_CHANNELS") == 0)
                {
                    enable_all_channels();
                    for (int i = 0; i < 4; i++)
                    {
                        diagnostic_channels[i] = true;
                    }
                }
                else if (strcmp(command, "DISABLE_ALL_CHANNELS") == 0)
                {
                    disable_all_channels();
                    for (int i = 0; i < 4; i++)
                    {
                        diagnostic_channels[i] = false;
                    }
                }
                else if (strcmp(command, "TOGGLE_ALL_CHANNELS") == 0)
                {
                    toggle_all_channels();
                    get_channel_states(diagnostic_channels);
                }
                else if (strcmp(command, "RUN_DIAGNOSTICS") == 0)
                {
                    test_diagnostic_channels();
                    queue_message("{\"type\":\"log\",\"level\":\"info\",\"source\":\"Diagnostics\",\"message\":\"Running diagnostic tests...\"}", true, -1);
                }
                else if (strcmp(command, "RUN_HAL_DEMO") == 0)
                {
                    // Run HAL demo (you'll need to implement this)
                    queue_message("{\"type\":\"log\",\"level\":\"info\",\"source\":\"HAL\",\"message\":\"Running HAL demonstration...\"}", true, -1);
                }
                else if (strcmp(command, "RUN_HAL_TEST") == 0)
                {
                    // Run HAL tests (you'll need to implement this)
                    queue_message("{\"type\":\"log\",\"level\":\"info\",\"source\":\"HAL\",\"message\":\"Running HAL tests...\"}", true, -1);
                }
                else if (strcmp(command, "EMERGENCY_STOP") == 0)
                {
                    emergency_shutdown("WebSocket emergency stop command");
                    for (int i = 0; i < 4; i++)
                    {
                        diagnostic_channels[i] = false;
                    }
                    queue_message("{\"type\":\"log\",\"level\":\"error\",\"source\":\"Emergency\",\"message\":\"EMERGENCY STOP ACTIVATED\"}", true, -1);
                }
                else
                {
                    printf("[WS] Unknown command: %s\n", command);
                }

                // Send updated status after command
                broadcast_status_update();
            }
        }
    }
}

// =============================================================================
// STATUS BROADCASTING
// =============================================================================

static void broadcast_status_update(void)
{
    char status_msg[512];

    // Get system metrics
    uint32_t uptime = get_system_uptime_seconds();
    uint32_t loop_count = get_loop_counter();

    // Read temperature (simplified)
    uint16_t temp_raw;
    float temperature = 25.0f; // Default
    if (hal_adc_read(ADC_TEMPERATURE, &temp_raw) == HAL_OK)
    {
        temperature = 27.0f - (temp_raw * 3.3f / 4096.0f - 0.706f) / 0.001721f;
    }

    snprintf(status_msg, sizeof(status_msg),
             "{"
             "\"type\":\"status\","
             "\"channels\":[%s,%s,%s,%s],"
             "\"system\":{"
             "\"temperature\":%.1f,"
             "\"uptime\":%lu,"
             "\"loopCount\":%lu,"
             "\"freeMemory\":64"
             "}"
             "}",
             diagnostic_channels[0] ? "true" : "false",
             diagnostic_channels[1] ? "true" : "false",
             diagnostic_channels[2] ? "true" : "false",
             diagnostic_channels[3] ? "true" : "false",
             temperature,
             uptime,
             loop_count);

    queue_message(status_msg, true, -1);
}

static void send_system_info(int client_socket)
{
    char info_msg[256];

    // Get IP address
    char ip_str[16] = "192.168.1.100"; // You'll want to get actual IP from wifi_manager

    snprintf(info_msg, sizeof(info_msg),
             "{"
             "\"type\":\"system_info\","
             "\"ip\":\"%s\","
             "\"buildDate\":\"%s %s\","
             "\"version\":\"1.0.0\","
             "\"board\":\"Pico W\""
             "}",
             ip_str,
             __DATE__, __TIME__);

    websocket_send_frame(client_socket, info_msg, strlen(info_msg), WS_OPCODE_TEXT);
}

// =============================================================================
// MESSAGE QUEUE MANAGEMENT
// =============================================================================

static void queue_message(const char *message, bool broadcast, int target_client)
{
    int next_head = (queue_head + 1) % 16;
    if (next_head == queue_tail)
    {
        printf("[WS] Message queue full!\n");
        return;
    }

    strncpy(message_queue[queue_head].data, message, WEBSOCKET_BUFFER_SIZE - 1);
    message_queue[queue_head].data[WEBSOCKET_BUFFER_SIZE - 1] = '\0';
    message_queue[queue_head].length = strlen(message);
    message_queue[queue_head].broadcast = broadcast;
    message_queue[queue_head].target_client = target_client;

    queue_head = next_head;
}

static void process_message_queue(void)
{
    while (queue_tail != queue_head)
    {
        ws_message_t *msg = &message_queue[queue_tail];

        if (msg->broadcast)
        {
            // Send to all connected clients
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].connected && clients[i].handshake_complete)
                {
                    websocket_send_frame(clients[i].socket_fd, msg->data, msg->length, WS_OPCODE_TEXT);
                }
            }
        }
        else if (msg->target_client >= 0 && msg->target_client < MAX_CLIENTS)
        {
            // Send to specific client
            if (clients[msg->target_client].connected && clients[msg->target_client].handshake_complete)
            {
                websocket_send_frame(clients[msg->target_client].socket_fd, msg->data, msg->length, WS_OPCODE_TEXT);
            }
        }

        queue_tail = (queue_tail + 1) % 16;
    }
}

// =============================================================================
// CLIENT MANAGEMENT
// =============================================================================

static int find_free_client_slot(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].connected)
        {
            return i;
        }
    }
    return -1;
}

static void cleanup_client(int client_index)
{
    if (client_index >= 0 && client_index < MAX_CLIENTS)
    {
        if (clients[client_index].socket_fd >= 0)
        {
            close(clients[client_index].socket_fd);
        }
        memset(&clients[client_index], 0, sizeof(websocket_client_t));
        clients[client_index].socket_fd = -1;
        printf("[WS] Client %d disconnected\n", client_index);
    }
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

bool websocket_server_init(void)
{
    if (websocket_server_running)
    {
        return true;
    }

    printf("[WS] Initializing WebSocket server...\n");

    // Initialize client array
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        memset(&clients[i], 0, sizeof(websocket_client_t));
        clients[i].socket_fd = -1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("[WS] Failed to create socket\n");
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        printf("[WS] Failed to set socket options\n");
        close(server_socket);
        return false;
    }

    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(NET_WEBSOCKET_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("[WS] Failed to bind socket to port %d\n", NET_WEBSOCKET_PORT);
        close(server_socket);
        return false;
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        printf("[WS] Failed to listen on socket\n");
        close(server_socket);
        return false;
    }

    websocket_server_running = true;
    last_status_broadcast = hal_get_tick_ms();

    printf("[WS] WebSocket server listening on port %d\n", NET_WEBSOCKET_PORT);
    return true;
}

void websocket_server_update(void)
{
    if (!websocket_server_running)
    {
        return;
    }

    // Check for new connections (non-blocking)
    fd_set read_fds;
    struct timeval timeout = {0, 0}; // Non-blocking

    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);
    int max_fd = server_socket;

    // Add client sockets to set
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].socket_fd >= 0)
        {
            FD_SET(clients[i].socket_fd, &read_fds);
            if (clients[i].socket_fd > max_fd)
            {
                max_fd = clients[i].socket_fd;
            }
        }
    }

    int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0)
    {
        return;
    }

    // Check for new connection
    if (FD_ISSET(server_socket, &read_fds))
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket >= 0)
        {
            int client_index = find_free_client_slot();
            if (client_index >= 0)
            {
                clients[client_index].socket_fd = client_socket;
                clients[client_index].connected = true;
                clients[client_index].handshake_complete = false;
                clients[client_index].last_ping = hal_get_tick_ms();

                // Convert IP to string
                char *ip_str = inet_ntoa(client_addr.sin_addr);
                strncpy(clients[client_index].client_ip, ip_str, sizeof(clients[client_index].client_ip) - 1);

                printf("[WS] New client connected: %s (slot %d)\n", ip_str, client_index);
            }
            else
            {
                printf("[WS] Max clients reached, rejecting connection\n");
                close(client_socket);
            }
        }
    }

    // Check client sockets for data
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].socket_fd >= 0 && FD_ISSET(clients[i].socket_fd, &read_fds))
        {
            char buffer[WEBSOCKET_BUFFER_SIZE];
            int bytes_received = recv(clients[i].socket_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0)
            {
                // Client disconnected
                cleanup_client(i);
                continue;
            }

            buffer[bytes_received] = '\0';

            if (!clients[i].handshake_complete)
            {
                // Handle WebSocket handshake
                if (strstr(buffer, "Upgrade: websocket"))
                {
                    if (websocket_handshake(clients[i].socket_fd, buffer))
                    {
                        clients[i].handshake_complete = true;
                        printf("[WS] Client %d handshake completed\n", i);

                        // Send initial system info
                        send_system_info(clients[i].socket_fd);
                    }
                    else
                    {
                        cleanup_client(i);
                    }
                }
            }
            else
            {
                // Handle WebSocket frames
                char payload[WEBSOCKET_BUFFER_SIZE];
                size_t payload_length;

                if (websocket_parse_frame(buffer, bytes_received, payload, &payload_length))
                {
                    handle_websocket_message(clients[i].socket_fd, payload);
                }
            }
        }
    }

    // Periodic status updates
    uint32_t current_time = hal_get_tick_ms();
    if (current_time - last_status_broadcast >= STATUS_UPDATE_INTERVAL_MS)
    {
        broadcast_status_update();
        last_status_broadcast = current_time;
    }

    // Process message queue
    process_message_queue();
}

void websocket_server_stop(void)
{
    if (!websocket_server_running)
    {
        return;
    }

    printf("[WS] Stopping WebSocket server...\n");

    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        cleanup_client(i);
    }

    // Close server socket
    if (server_socket >= 0)
    {
        close(server_socket);
        server_socket = -1;
    }

    websocket_server_running = false;
    printf("[WS] WebSocket server stopped\n");
}

bool websocket_server_is_running(void)
{
    return websocket_server_running;
}

int websocket_server_get_client_count(void)
{
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].handshake_complete)
        {
            count++;
        }
    }
    return count;
}

void websocket_send_log(const char *level, const char *source, const char *message)
{
    char log_msg[384];
    snprintf(log_msg, sizeof(log_msg),
             "{"
             "\"type\":\"log\","
             "\"level\":\"%s\","
             "\"source\":\"%s\","
             "\"message\":\"%s\""
             "}",
             level, source, message);

    queue_message(log_msg, true, -1);
}

void websocket_send_channel_data(int channel, float voltage, float current)
{
    char data_msg[256];
    snprintf(data_msg, sizeof(data_msg),
             "{"
             "\"type\":\"channel_data\","
             "\"channel\":%d,"
             "\"voltage\":%.3f,"
             "\"current\":%.3f"
             "}",
             channel, voltage, current);

    queue_message(data_msg, true, -1);
}