/**
 * @file wifi_manager.cpp
 * @brief WiFi management implementation for Pico W
 *
 * This file provides WiFi connection management, including connection,
 * disconnection, status monitoring, and network configuration.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "wifi_manager.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool wifi_initialized = false;
static bool wifi_connected = false;
static wifi_config_t current_config;
static wifi_status_t wifi_status = WIFI_STATUS_DISCONNECTED;
static uint32_t connection_start_time = 0;
static uint32_t last_status_check = 0;
static uint32_t connection_attempts = 0;
static char current_ip[16] = {0};

// Callback functions
static wifi_event_callback_t event_callback = NULL;
static wifi_scan_callback_t scan_callback = NULL;

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static void wifi_status_monitor(void);
static void update_ip_address(void);
static const char *wifi_status_to_string(wifi_status_t status);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

bool wifi_manager_init(void)
{
    if (wifi_initialized)
    {
        return true;
    }

    printf("[WiFi] Initializing WiFi manager...\n");

    // Initialize CYW43 architecture
    if (cyw43_arch_init() != 0)
    {
        printf("[WiFi] Failed to initialize CYW43 architecture\n");
        return false;
    }

    // Enable WiFi station mode
    cyw43_arch_enable_sta_mode();

    // Initialize default configuration
    memset(&current_config, 0, sizeof(current_config));
    strncpy(current_config.hostname, WIFI_HOSTNAME, sizeof(current_config.hostname) - 1);
    current_config.connect_timeout_ms = WIFI_CONNECT_TIMEOUT_MS;
    current_config.reconnect_delay_ms = WIFI_RECONNECT_DELAY_MS;
    current_config.auto_reconnect = true;

    wifi_status = WIFI_STATUS_DISCONNECTED;
    wifi_initialized = true;
    last_status_check = hal_get_tick_ms();

    printf("[WiFi] WiFi manager initialized\n");
    return true;
}

void wifi_manager_deinit(void)
{
    if (!wifi_initialized)
    {
        return;
    }

    printf("[WiFi] Deinitializing WiFi manager...\n");

    if (wifi_connected)
    {
        wifi_disconnect();
    }

    cyw43_arch_deinit();
    wifi_initialized = false;
    wifi_connected = false;
    wifi_status = WIFI_STATUS_DISCONNECTED;

    printf("[WiFi] WiFi manager deinitialized\n");
}

bool wifi_connect(const char *ssid, const char *password)
{
    if (!wifi_initialized)
    {
        printf("[WiFi] WiFi manager not initialized\n");
        return false;
    }

    if (!ssid || strlen(ssid) == 0)
    {
        printf("[WiFi] Invalid SSID\n");
        return false;
    }

    if (wifi_connected)
    {
        wifi_disconnect();
    }

    printf("[WiFi] Connecting to SSID: %s\n", ssid);

    // Store connection parameters
    strncpy(current_config.ssid, ssid, sizeof(current_config.ssid) - 1);
    if (password)
    {
        strncpy(current_config.password, password, sizeof(current_config.password) - 1);
    }
    else
    {
        current_config.password[0] = '\0';
    }

    wifi_status = WIFI_STATUS_CONNECTING;
    connection_start_time = hal_get_tick_ms();
    connection_attempts++;

    // Trigger event callback
    if (event_callback)
    {
        event_callback(WIFI_EVENT_CONNECTING, &current_config);
    }

    // Attempt connection
    int result;
    if (password && strlen(password) > 0)
    {
        result = cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK,
                                                    current_config.connect_timeout_ms);
    }
    else
    {
        result = cyw43_arch_wifi_connect_timeout_ms(ssid, NULL, CYW43_AUTH_OPEN,
                                                    current_config.connect_timeout_ms);
    }

    if (result == 0)
    {
        wifi_connected = true;
        wifi_status = WIFI_STATUS_CONNECTED;
        update_ip_address();

        printf("[WiFi] Connected successfully to %s\n", ssid);
        printf("[WiFi] IP Address: %s\n", current_ip);

        if (event_callback)
        {
            event_callback(WIFI_EVENT_CONNECTED, &current_config);
        }

        return true;
    }
    else
    {
        wifi_status = WIFI_STATUS_CONNECTION_FAILED;
        printf("[WiFi] Connection failed (error: %d)\n", result);

        if (event_callback)
        {
            event_callback(WIFI_EVENT_CONNECTION_FAILED, &current_config);
        }

        return false;
    }
}

void wifi_disconnect(void)
{
    if (!wifi_initialized || !wifi_connected)
    {
        return;
    }

    printf("[WiFi] Disconnecting from WiFi...\n");

    wifi_status = WIFI_STATUS_DISCONNECTING;

    // Disconnect from WiFi
    cyw43_arch_wifi_disconnect();

    wifi_connected = false;
    wifi_status = WIFI_STATUS_DISCONNECTED;
    current_ip[0] = '\0';

    printf("[WiFi] Disconnected from WiFi\n");

    if (event_callback)
    {
        event_callback(WIFI_EVENT_DISCONNECTED, &current_config);
    }
}

wifi_status_t wifi_get_status(void)
{
    return wifi_status;
}

bool wifi_is_connected(void)
{
    return wifi_connected && (wifi_status == WIFI_STATUS_CONNECTED);
}

const char *wifi_get_ip_address(void)
{
    if (wifi_is_connected())
    {
        return current_ip;
    }
    return "";
}

int wifi_get_rssi(void)
{
    if (!wifi_is_connected())
    {
        return -100; // No signal
    }

    // Get signal strength (simplified - actual implementation would query CYW43)
    // This is a placeholder - you'd need to implement actual RSSI reading
    return -45; // Example value
}

const char *wifi_get_ssid(void)
{
    if (wifi_is_connected())
    {
        return current_config.ssid;
    }
    return "";
}

void wifi_manager_update(void)
{
    if (!wifi_initialized)
    {
        return;
    }

    uint32_t current_time = hal_get_tick_ms();

    // Check status periodically (every 5 seconds)
    if (current_time - last_status_check >= 5000)
    {
        wifi_status_monitor();
        last_status_check = current_time;
    }

    // Handle auto-reconnection
    if (current_config.auto_reconnect &&
        wifi_status == WIFI_STATUS_CONNECTION_FAILED &&
        strlen(current_config.ssid) > 0)
    {

        static uint32_t last_reconnect_attempt = 0;
        if (current_time - last_reconnect_attempt >= current_config.reconnect_delay_ms)
        {
            printf("[WiFi] Attempting auto-reconnection...\n");
            wifi_connect(current_config.ssid, current_config.password);
            last_reconnect_attempt = current_time;
        }
    }
}

bool wifi_scan_networks(void)
{
    if (!wifi_initialized)
    {
        printf("[WiFi] WiFi manager not initialized\n");
        return false;
    }

    printf("[WiFi] Starting network scan...\n");

    // Note: This is a simplified implementation
    // A full implementation would use cyw43_wifi_scan() and handle results asynchronously

    // For now, we'll simulate finding some networks
    if (scan_callback)
    {
        wifi_scan_result_t results[] = {
            {"HomeNetwork", -45, true, CYW43_AUTH_WPA2_AES_PSK},
            {"OpenWiFi", -60, false, CYW43_AUTH_OPEN},
            {"Office5G", -50, true, CYW43_AUTH_WPA2_AES_PSK}};

        for (int i = 0; i < 3; i++)
        {
            scan_callback(&results[i], i == 2); // Last result
        }
    }

    return true;
}

void wifi_set_config(const wifi_config_t *config)
{
    if (!config)
    {
        return;
    }

    memcpy(&current_config, config, sizeof(wifi_config_t));
    printf("[WiFi] Configuration updated\n");
}

void wifi_get_config(wifi_config_t *config)
{
    if (!config)
    {
        return;
    }

    memcpy(config, &current_config, sizeof(wifi_config_t));
}

void wifi_register_event_callback(wifi_event_callback_t callback)
{
    event_callback = callback;
}

void wifi_register_scan_callback(wifi_scan_callback_t callback)
{
    scan_callback = callback;
}

void wifi_get_statistics(wifi_statistics_t *stats)
{
    if (!stats)
    {
        return;
    }

    memset(stats, 0, sizeof(wifi_statistics_t));

    stats->connection_attempts = connection_attempts;
    stats->is_connected = wifi_is_connected();
    stats->signal_strength = wifi_get_rssi();

    if (wifi_is_connected() && connection_start_time > 0)
    {
        stats->uptime_ms = hal_get_tick_ms() - connection_start_time;
    }

    strncpy(stats->current_ssid, current_config.ssid, sizeof(stats->current_ssid) - 1);
    strncpy(stats->ip_address, current_ip, sizeof(stats->ip_address) - 1);
}

const char *wifi_get_status_string(void)
{
    return wifi_status_to_string(wifi_status);
}

void wifi_set_hostname(const char *hostname)
{
    if (!hostname)
    {
        return;
    }

    strncpy(current_config.hostname, hostname, sizeof(current_config.hostname) - 1);
    current_config.hostname[sizeof(current_config.hostname) - 1] = '\0';

    printf("[WiFi] Hostname set to: %s\n", hostname);
}

void wifi_enable_auto_reconnect(bool enable)
{
    current_config.auto_reconnect = enable;
    printf("[WiFi] Auto-reconnect %s\n", enable ? "enabled" : "disabled");
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

static void wifi_status_monitor(void)
{
    if (!wifi_initialized)
    {
        return;
    }

    // Check actual WiFi link status
    int link_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);

    wifi_status_t old_status = wifi_status;

    switch (link_status)
    {
    case CYW43_LINK_DOWN:
        if (wifi_connected)
        {
            wifi_connected = false;
            wifi_status = WIFI_STATUS_DISCONNECTED;
            current_ip[0] = '\0';
            printf("[WiFi] Link down detected\n");
        }
        break;

    case CYW43_LINK_JOIN:
        wifi_status = WIFI_STATUS_CONNECTING;
        break;

    case CYW43_LINK_UP:
        if (!wifi_connected)
        {
            wifi_connected = true;
            wifi_status = WIFI_STATUS_CONNECTED;
            update_ip_address();
            printf("[WiFi] Link up detected - IP: %s\n", current_ip);
        }
        break;

    case CYW43_LINK_FAIL:
    case CYW43_LINK_NONET:
    case CYW43_LINK_BADAUTH:
        wifi_connected = false;
        wifi_status = WIFI_STATUS_CONNECTION_FAILED;
        current_ip[0] = '\0';
        break;

    default:
        break;
    }

    // Trigger event callback if status changed
    if (old_status != wifi_status && event_callback)
    {
        switch (wifi_status)
        {
        case WIFI_STATUS_CONNECTED:
            event_callback(WIFI_EVENT_CONNECTED, &current_config);
            break;
        case WIFI_STATUS_DISCONNECTED:
            event_callback(WIFI_EVENT_DISCONNECTED, &current_config);
            break;
        case WIFI_STATUS_CONNECTION_FAILED:
            event_callback(WIFI_EVENT_CONNECTION_FAILED, &current_config);
            break;
        default:
            break;
        }
    }
}

static void update_ip_address(void)
{
    if (!wifi_connected)
    {
        current_ip[0] = '\0';
        return;
    }

    // Get IP address from network interface
    // Note: This is simplified - you'd typically use netif APIs
    ip4_addr_t ip = cyw43_state.netif[CYW43_ITF_STA].ip_addr;

    snprintf(current_ip, sizeof(current_ip), "%d.%d.%d.%d",
             ip4_addr1(&ip), ip4_addr2(&ip), ip4_addr3(&ip), ip4_addr4(&ip));
}

static const char *wifi_status_to_string(wifi_status_t status)
{
    switch (status)
    {
    case WIFI_STATUS_DISCONNECTED:
        return "Disconnected";
    case WIFI_STATUS_CONNECTING:
        return "Connecting";
    case WIFI_STATUS_CONNECTED:
        return "Connected";
    case WIFI_STATUS_DISCONNECTING:
        return "Disconnecting";
    case WIFI_STATUS_CONNECTION_FAILED:
        return "Connection Failed";
    case WIFI_STATUS_SCANNING:
        return "Scanning";
    default:
        return "Unknown";
    }
}