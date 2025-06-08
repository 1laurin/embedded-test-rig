/**
 * @file wifi_manager.cpp
 * @brief Simplified WiFi manager implementation for Raspberry Pi Pico W
 */

#include "../include/wifi_manager.h"
#include "../include/board_config.h"

// Pico W WiFi includes
#include "pico/cyw43_arch.h"

#include <cstring>
#include <cstdio>

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static bool wifi_initialized = false;
static bool wifi_connected = false;
static char current_ssid[WIFI_SSID_MAX_LENGTH] = {0};
static char current_ip[16] = {0};
static int current_rssi = 0;
static uint32_t connection_start_time = 0;

// =============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// =============================================================================

bool wifi_manager_init(void)
{
    if (wifi_initialized)
    {
        return true;
    }

    // CYW43 should already be initialized in main.cpp
    wifi_initialized = true;
    printf("[WIFI] WiFi manager initialized\n");
    return true;
}

void wifi_manager_deinit(void)
{
    if (wifi_connected)
    {
        wifi_disconnect();
    }
    wifi_initialized = false;
    printf("[WIFI] WiFi manager deinitialized\n");
}

bool wifi_connect(const char *ssid, const char *password)
{
    if (!wifi_initialized)
    {
        printf("[WIFI] WiFi manager not initialized\n");
        return false;
    }

    if (!ssid)
    {
        printf("[WIFI] SSID cannot be NULL\n");
        return false;
    }

    printf("[WIFI] Connecting to %s...\n", ssid);

    // Store SSID
    strncpy(current_ssid, ssid, sizeof(current_ssid) - 1);
    current_ssid[sizeof(current_ssid) - 1] = '\0';

    // Attempt connection
    int result;
    if (password && strlen(password) > 0)
    {
        result = cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECT_TIMEOUT_MS);
    }
    else
    {
        result = cyw43_arch_wifi_connect_timeout_ms(ssid, NULL, CYW43_AUTH_OPEN, WIFI_CONNECT_TIMEOUT_MS);
    }

    if (result == 0)
    {
        wifi_connected = true;

        // Get IP address
        const ip_addr_t *ip = netif_ip_addr4(netif_default);
        if (ip)
        {
            snprintf(current_ip, sizeof(current_ip), "%s", ip4addr_ntoa(ip));
        }

        printf("[WIFI] Connected successfully\n");
        printf("[WIFI] IP Address: %s\n", current_ip);
        return true;
    }
    else
    {
        printf("[WIFI] Connection failed with error: %d\n", result);
        return false;
    }
}

void wifi_disconnect(void)
{
    if (!wifi_initialized)
    {
        return;
    }

    if (wifi_connected)
    {
        // Note: cyw43_arch_wifi_disconnect() doesn't exist in the SDK
        // We'll set the status and let the connection timeout
        wifi_connected = false;
        memset(current_ip, 0, sizeof(current_ip));
        printf("[WIFI] Disconnected\n");
    }
}

bool wifi_is_connected(void)
{
    if (!wifi_initialized)
    {
        return false;
    }

    // Check actual WiFi status
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    bool actually_connected = (status == CYW43_LINK_UP);

    if (wifi_connected != actually_connected)
    {
        wifi_connected = actually_connected;
        if (!wifi_connected)
        {
            memset(current_ip, 0, sizeof(current_ip));
        }
    }

    return wifi_connected;
}

const char *wifi_get_ip_address(void)
{
    if (!wifi_is_connected())
    {
        return "0.0.0.0";
    }

    // Update IP address if needed
    if (strlen(current_ip) == 0)
    {
        const ip_addr_t *ip = netif_ip_addr4(netif_default);
        if (ip)
        {
            snprintf(current_ip, sizeof(current_ip), "%s", ip4addr_ntoa(ip));
        }
    }

    return current_ip;
}

const char *wifi_get_ssid(void)
{
    return current_ssid;
}

int wifi_get_rssi(void)
{
    // This is a simplified implementation
    // The actual RSSI would need to be read from the CYW43 chip
    return current_rssi;
}

void wifi_manager_update(void)
{
    if (!wifi_initialized)
    {
        return;
    }

    // Check connection status
    wifi_is_connected();
}

void wifi_set_hostname(const char *hostname)
{
    if (!hostname)
    {
        return;
    }

    // Set the hostname for DHCP
    netif_set_hostname(netif_default, hostname);
    printf("[WIFI] Hostname set to: %s\n", hostname);
}

// Helper functions for the macros in board_config.h
bool wifi_is_connected_helper(void)
{
    return wifi_is_connected();
}

void wifi_set_led(bool state)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
}

void wifi_toggle_led(void)
{
    static bool led_state = false;
    led_state = !led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
}

// Stub functions for compatibility
void wifi_register_event_callback(void *callback)
{
    // Simplified - no event callbacks in this version
    printf("[WIFI] Event callback registered (simplified implementation)\n");
}

bool wifi_scan_networks(void)
{
    // Simplified - no network scanning in this version
    printf("[WIFI] Network scan not implemented in simplified version\n");
    return false;
}

void wifi_register_scan_callback(void *callback)
{
    // Simplified - no scan callbacks in this version
    printf("[WIFI] Scan callback registered (simplified implementation)\n");
}