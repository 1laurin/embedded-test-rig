/**
 * @file websocket_server.cpp
 * @brief Simplified WebSocket server implementation for Raspberry Pi Pico W
 *
 * This is a simplified version that works with lwIP and focuses on basic functionality.
 */

#include "websocket_server.h"
#include "board_config.h"

// lwIP includes for networking
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/altcp.h"

#include <cstring>
#include <cstdio>

// =============================================================================
// PRIVATE CONSTANTS
// =============================================================================

#define MAX_WEBSOCKET_CLIENTS 4
#define WEBSOCKET_BUFFER_SIZE 512
#define HTTP_RESPONSE_SIZE 1024

// =============================================================================
// PRIVATE TYPES
// =============================================================================

typedef struct
{
    struct tcp_pcb *pcb;
    bool connected;
    bool websocket_handshake_complete;
    char client_ip[16];
    uint32_t last_activity;
} websocket_client_t;

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static struct tcp_pcb *websocket_server_pcb = NULL;
static websocket_client_t clients[MAX_WEBSOCKET_CLIENTS];
static bool server_initialized = false;

// Callback function pointers
static websocket_command_callback_t command_callback = NULL;
static websocket_client_callback_t client_callback = NULL;

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static err_t websocket_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t websocket_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void websocket_err(void *arg, err_t err);
static err_t websocket_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static bool handle_http_request(struct tcp_pcb *pcb, const char *request);
static bool handle_websocket_frame(int client_index, const char *data, size_t len);
static void send_websocket_response(struct tcp_pcb *pcb, const char *key);
static int find_free_client_slot(void);
static int find_client_by_pcb(struct tcp_pcb *pcb);
static void cleanup_client_slot(int index);

// =============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// =============================================================================

bool websocket_server_init(void)
{
    if (server_initialized)
    {
        return true;
    }

    // Initialize client slots
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        clients[i].pcb = NULL;
        clients[i].connected = false;
        clients[i].websocket_handshake_complete = false;
        memset(clients[i].client_ip, 0, sizeof(clients[i].client_ip));
        clients[i].last_activity = 0;
    }

    // Create TCP PCB for the server
    websocket_server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (websocket_server_pcb == NULL)
    {
        printf("[WEBSOCKET] Failed to create server PCB\n");
        return false;
    }

    // Bind to the websocket port
    err_t err = tcp_bind(websocket_server_pcb, IP_ANY_TYPE, NET_WEBSOCKET_PORT);
    if (err != ERR_OK)
    {
        printf("[WEBSOCKET] Failed to bind to port %d: %d\n", NET_WEBSOCKET_PORT, err);
        tcp_close(websocket_server_pcb);
        websocket_server_pcb = NULL;
        return false;
    }

    // Start listening
    websocket_server_pcb = tcp_listen(websocket_server_pcb);
    if (websocket_server_pcb == NULL)
    {
        printf("[WEBSOCKET] Failed to listen on port %d\n", NET_WEBSOCKET_PORT);
        return false;
    }

    // Set accept callback
    tcp_accept(websocket_server_pcb, websocket_accept);

    server_initialized = true;
    printf("[WEBSOCKET] Server started on port %d\n", NET_WEBSOCKET_PORT);
    return true;
}

void websocket_server_update(void)
{
    // This function is called periodically to handle cleanup and maintenance
    // With lwIP, most of the work is done in callbacks, so this is minimal

    if (!server_initialized)
    {
        return;
    }

    // Check for inactive clients and clean them up
    uint32_t current_time = 0; // You'd get this from your timer system

    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].pcb)
        {
            // Check for timeout (simplified)
            // if (current_time - clients[i].last_activity > WEBSOCKET_TIMEOUT_MS) {
            //     cleanup_client_slot(i);
            // }
        }
    }
}

void websocket_server_stop(void)
{
    if (!server_initialized)
    {
        return;
    }

    // Close all client connections
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (clients[i].pcb)
        {
            tcp_close(clients[i].pcb);
            cleanup_client_slot(i);
        }
    }

    // Close server
    if (websocket_server_pcb)
    {
        tcp_close(websocket_server_pcb);
        websocket_server_pcb = NULL;
    }

    server_initialized = false;
    printf("[WEBSOCKET] Server stopped\n");
}

void websocket_register_command_callback(websocket_command_callback_t callback)
{
    command_callback = callback;
}

void websocket_register_client_callback(websocket_client_callback_t callback)
{
    client_callback = callback;
}

void websocket_send_log(const char *level, const char *category, const char *message)
{
    if (!server_initialized)
    {
        return;
    }

    char json_msg[256];
    snprintf(json_msg, sizeof(json_msg),
             "{\"type\":\"log\",\"level\":\"%s\",\"category\":\"%s\",\"message\":\"%s\"}",
             level, category, message);

    // Send to all connected WebSocket clients
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].websocket_handshake_complete && clients[i].pcb)
        {
            // Send WebSocket frame (simplified)
            tcp_write(clients[i].pcb, json_msg, strlen(json_msg), TCP_WRITE_FLAG_COPY);
            tcp_output(clients[i].pcb);
        }
    }
}

void websocket_send_channel_data(int channel, float voltage, float current)
{
    if (!server_initialized)
    {
        return;
    }

    char json_msg[128];
    snprintf(json_msg, sizeof(json_msg),
             "{\"type\":\"channel_data\",\"channel\":%d,\"voltage\":%.2f,\"current\":%.3f}",
             channel, voltage, current);

    // Send to all connected WebSocket clients
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (clients[i].connected && clients[i].websocket_handshake_complete && clients[i].pcb)
        {
            tcp_write(clients[i].pcb, json_msg, strlen(json_msg), TCP_WRITE_FLAG_COPY);
            tcp_output(clients[i].pcb);
        }
    }
}

// =============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// =============================================================================

static err_t websocket_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    if (err != ERR_OK || newpcb == NULL)
    {
        return ERR_VAL;
    }

    // Find a free client slot
    int client_index = find_free_client_slot();
    if (client_index == -1)
    {
        printf("[WEBSOCKET] No free client slots, rejecting connection\n");
        tcp_close(newpcb);
        return ERR_MEM;
    }

    // Set up client
    clients[client_index].pcb = newpcb;
    clients[client_index].connected = true;
    clients[client_index].websocket_handshake_complete = false;

    // Get client IP (simplified)
    snprintf(clients[client_index].client_ip, sizeof(clients[client_index].client_ip),
             "%s", ipaddr_ntoa(&newpcb->remote_ip));

    // Set callbacks
    tcp_arg(newpcb, (void *)(intptr_t)client_index);
    tcp_recv(newpcb, websocket_recv);
    tcp_err(newpcb, websocket_err);
    tcp_sent(newpcb, websocket_sent);

    printf("[WEBSOCKET] Client %d connected from %s\n", client_index, clients[client_index].client_ip);

    // Notify callback
    if (client_callback)
    {
        client_callback(client_index, true, clients[client_index].client_ip);
    }

    return ERR_OK;
}

static err_t websocket_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    int client_index = (int)(intptr_t)arg;

    if (err != ERR_OK)
    {
        if (p)
            pbuf_free(p);
        return err;
    }

    if (p == NULL)
    {
        // Connection closed
        cleanup_client_slot(client_index);
        return ERR_OK;
    }

    // Copy data from pbuf
    char buffer[WEBSOCKET_BUFFER_SIZE];
    size_t len = pbuf_copy_partial(p, buffer, sizeof(buffer) - 1, 0);
    buffer[len] = '\0';

    // Free the pbuf
    pbuf_free(p);

    // Tell TCP that we've received the data
    tcp_recved(tpcb, len);

    // Handle the request
    if (!clients[client_index].websocket_handshake_complete)
    {
        // This is an HTTP request for WebSocket upgrade
        if (strstr(buffer, "Upgrade: websocket") != NULL)
        {
            // Extract WebSocket key and send response
            char *key_start = strstr(buffer, "Sec-WebSocket-Key:");
            if (key_start)
            {
                key_start += 18; // Skip "Sec-WebSocket-Key:"
                while (*key_start == ' ')
                    key_start++; // Skip spaces
                char *key_end = strstr(key_start, "\r\n");
                if (key_end)
                {
                    *key_end = '\0';
                    send_websocket_response(tpcb, key_start);
                    clients[client_index].websocket_handshake_complete = true;
                }
            }
        }
        else
        {
            // Regular HTTP request
            handle_http_request(tpcb, buffer);
        }
    }
    else
    {
        // This is a WebSocket frame
        handle_websocket_frame(client_index, buffer, len);
    }

    return ERR_OK;
}

static void websocket_err(void *arg, err_t err)
{
    int client_index = (int)(intptr_t)arg;
    printf("[WEBSOCKET] Error on client %d: %d\n", client_index, err);
    cleanup_client_slot(client_index);
}

static err_t websocket_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    // Data has been sent successfully
    return ERR_OK;
}

static bool handle_http_request(struct tcp_pcb *pcb, const char *request)
{
    // Simple HTTP response for non-WebSocket requests
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 100\r\n"
        "\r\n"
        "<html><body><h1>Pico W Diagnostic Rig</h1><p>WebSocket server is running on port 8080</p></body></html>";

    tcp_write(pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    return true;
}

static void send_websocket_response(struct tcp_pcb *pcb, const char *key)
{
    // Simplified WebSocket handshake response
    const char *response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: dGhlIHNhbXBsZSBub25jZQ==\r\n" // Simplified
        "\r\n";

    tcp_write(pcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
}

static bool handle_websocket_frame(int client_index, const char *data, size_t len)
{
    // Simplified frame parsing - just look for JSON commands
    if (len > 2 && data[0] == '{')
    {
        // This looks like a JSON command
        printf("[WEBSOCKET] Received command from client %d: %.*s\n", client_index, (int)len, data);

        if (command_callback)
        {
            command_callback("test_command", "test_params", client_index);
        }
        return true;
    }

    return false;
}

static int find_free_client_slot(void)
{
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (!clients[i].connected)
        {
            return i;
        }
    }
    return -1;
}

static int find_client_by_pcb(struct tcp_pcb *pcb)
{
    for (int i = 0; i < MAX_WEBSOCKET_CLIENTS; i++)
    {
        if (clients[i].pcb == pcb)
        {
            return i;
        }
    }
    return -1;
}

static void cleanup_client_slot(int index)
{
    if (index >= 0 && index < MAX_WEBSOCKET_CLIENTS)
    {
        if (clients[index].connected && client_callback)
        {
            client_callback(index, false, clients[index].client_ip);
        }

        clients[index].pcb = NULL;
        clients[index].connected = false;
        clients[index].websocket_handshake_complete = false;
        memset(clients[index].client_ip, 0, sizeof(clients[index].client_ip));
        clients[index].last_activity = 0;
    }
}