/**
 * @file wifi_manager.h
 * @brief WiFi management interface for Pico W
 *
 * This file provides WiFi connection management including connection,
 * disconnection, status monitoring, and network configuration.
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "board_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // CONSTANTS
    // =============================================================================

#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define WIFI_HOSTNAME_MAX_LENGTH 32
#define WIFI_IP_ADDRESS_LENGTH 16

    // =============================================================================
    // ENUMERATIONS
    // =============================================================================

    /**
     * @brief WiFi connection status
     */
    typedef enum
    {
        WIFI_STATUS_DISCONNECTED = 0,
        WIFI_STATUS_CONNECTING,
        WIFI_STATUS_CONNECTED,
        WIFI_STATUS_DISCONNECTING,
        WIFI_STATUS_CONNECTION_FAILED,
        WIFI_STATUS_SCANNING
    } wifi_status_t;

    /**
     * @brief WiFi events
     */
    typedef enum
    {
        WIFI_EVENT_CONNECTING = 0,
        WIFI_EVENT_CONNECTED,
        WIFI_EVENT_DISCONNECTED,
        WIFI_EVENT_CONNECTION_FAILED,
        WIFI_EVENT_SCAN_COMPLETE,
        WIFI_EVENT_IP_ASSIGNED
    } wifi_event_t;

    /**
     * @brief WiFi authentication types
     */
    typedef enum
    {
        WIFI_AUTH_OPEN = 0,
        WIFI_AUTH_WEP,
        WIFI_AUTH_WPA_PSK,
        WIFI_AUTH_WPA2_PSK,
        WIFI_AUTH_WPA_WPA2_PSK,
        WIFI_AUTH_WPA2_ENTERPRISE
    } wifi_auth_t;

    // =============================================================================
    // STRUCTURES
    // =============================================================================

    /**
     * @brief WiFi configuration structure
     */
    typedef struct
    {
        char ssid[WIFI_SSID_MAX_LENGTH];
        char password[WIFI_PASSWORD_MAX_LENGTH];
        char hostname[WIFI_HOSTNAME_MAX_LENGTH];
        uint32_t connect_timeout_ms;
        uint32_t reconnect_delay_ms;
        bool auto_reconnect;
        bool dhcp_enabled;
        char static_ip[WIFI_IP_ADDRESS_LENGTH];
        char gateway[WIFI_IP_ADDRESS_LENGTH];
        char subnet_mask[WIFI_IP_ADDRESS_LENGTH];
        char dns1[WIFI_IP_ADDRESS_LENGTH];
        char dns2[WIFI_IP_ADDRESS_LENGTH];
    } wifi_config_t;

    /**
     * @brief WiFi scan result structure
     */
    typedef struct
    {
        char ssid[WIFI_SSID_MAX_LENGTH];
        int8_t rssi;
        bool security_enabled;
        wifi_auth_t auth_type;
        uint8_t channel;
        uint8_t bssid[6];
    } wifi_scan_result_t;

    /**
     * @brief WiFi statistics structure
     */
    typedef struct
    {
        uint32_t connection_attempts;
        uint32_t successful_connections;
        uint32_t failed_connections;
        uint32_t disconnections;
        uint32_t uptime_ms;
        uint32_t bytes_sent;
        uint32_t bytes_received;
        int8_t signal_strength;
        bool is_connected;
        char current_ssid[WIFI_SSID_MAX_LENGTH];
        char ip_address[WIFI_IP_ADDRESS_LENGTH];
        char mac_address[18];
    } wifi_statistics_t;

    // =============================================================================
    // CALLBACK TYPES
    // =============================================================================

    /**
     * @brief WiFi event callback function type
     * @param event WiFi event that occurred
     * @param config Current WiFi configuration
     */
    typedef void (*wifi_event_callback_t)(wifi_event_t event, const wifi_config_t *config);

    /**
     * @brief WiFi scan result callback function type
     * @param result Scan result information
     * @param is_last true if this is the last result in the scan
     */
    typedef void (*wifi_scan_callback_t)(const wifi_scan_result_t *result, bool is_last);

    // =============================================================================
    // PUBLIC FUNCTIONS
    // =============================================================================

    /**
     * @brief Initialize the WiFi manager
     * @return true on success, false on failure
     */
    bool wifi_manager_init(void);

    /**
     * @brief Deinitialize the WiFi manager and cleanup resources
     */
    void wifi_manager_deinit(void);

    /**
     * @brief Connect to a WiFi network
     * @param ssid Network SSID
     * @param password Network password (NULL for open networks)
     * @return true if connection initiated successfully, false otherwise
     */
    bool wifi_connect(const char *ssid, const char *password);

    /**
     * @brief Disconnect from current WiFi network
     */
    void wifi_disconnect(void);

    /**
     * @brief Get current WiFi connection status
     * @return Current WiFi status
     */
    wifi_status_t wifi_get_status(void);

    /**
     * @brief Check if WiFi is currently connected
     * @return true if connected, false otherwise
     */
    bool wifi_is_connected(void);

    /**
     * @brief Get current IP address
     * @return IP address string, or empty string if not connected
     */
    const char *wifi_get_ip_address(void);

    /**
     * @brief Get current signal strength (RSSI)
     * @return Signal strength in dBm, or -100 if not connected
     */
    int wifi_get_rssi(void);

    /**
     * @brief Get current connected SSID
     * @return SSID string, or empty string if not connected
     */
    const char *wifi_get_ssid(void);

    /**
     * @brief Update WiFi manager (call regularly from main loop)
     * This handles status monitoring, auto-reconnection, and other periodic tasks
     */
    void wifi_manager_update(void);

    /**
     * @brief Start scanning for available WiFi networks
     * @return true if scan started successfully, false otherwise
     */
    bool wifi_scan_networks(void);

    /**
     * @brief Set WiFi configuration
     * @param config WiFi configuration structure
     */
    void wifi_set_config(const wifi_config_t *config);

    /**
     * @brief Get current WiFi configuration
     * @param config Buffer to store current configuration
     */
    void wifi_get_config(wifi_config_t *config);

    /**
     * @brief Register callback for WiFi events
     * @param callback Function to call when WiFi events occur
     */
    void wifi_register_event_callback(wifi_event_callback_t callback);

    /**
     * @brief Register callback for WiFi scan results
     * @param callback Function to call with scan results
     */
    void wifi_register_scan_callback(wifi_scan_callback_t callback);

    /**
     * @brief Get WiFi statistics
     * @param stats Buffer to store statistics
     */
    void wifi_get_statistics(wifi_statistics_t *stats);

    /**
     * @brief Get WiFi status as a human-readable string
     * @return Status string
     */
    const char *wifi_get_status_string(void);

    /**
     * @brief Set device hostname
     * @param hostname Hostname string
     */
    void wifi_set_hostname(const char *hostname);

    /**
     * @brief Enable or disable auto-reconnection
     * @param enable true to enable, false to disable
     */
    void wifi_enable_auto_reconnect(bool enable);

    /**
     * @brief Reset WiFi configuration to defaults
     */
    void wifi_reset_config(void);

    /**
     * @brief Save WiFi configuration to flash memory
     * @return true on success, false on failure
     */
    bool wifi_save_config(void);

    /**
     * @brief Load WiFi configuration from flash memory
     * @return true on success, false on failure
     */
    bool wifi_load_config(void);

    /**
     * @brief Get MAC address
     * @param mac_address Buffer to store MAC address (18 bytes: "XX:XX:XX:XX:XX:XX")
     * @return true on success, false on failure
     */
    bool wifi_get_mac_address(char *mac_address);

    /**
     * @brief Set static IP configuration
     * @param ip IP address
     * @param gateway Gateway address
     * @param subnet Subnet mask
     * @param dns1 Primary DNS server
     * @param dns2 Secondary DNS server (optional)
     * @return true on success, false on failure
     */
    bool wifi_set_static_ip(const char *ip, const char *gateway, const char *subnet,
                            const char *dns1, const char *dns2);

    /**
     * @brief Enable DHCP mode
     * @return true on success, false on failure
     */
    bool wifi_enable_dhcp(void);

    /**
     * @brief Check if network is reachable
     * @param host Hostname or IP address to ping
     * @param timeout_ms Timeout in milliseconds
     * @return true if reachable, false otherwise
     */
    bool wifi_ping_host(const char *host, uint32_t timeout_ms);

    /**
     * @brief Get network information
     * @param gateway Buffer to store gateway IP
     * @param subnet Buffer to store subnet mask
     * @param dns1 Buffer to store primary DNS
     * @param dns2 Buffer to store secondary DNS
     * @return true on success, false on failure
     */
    bool wifi_get_network_info(char *gateway, char *subnet, char *dns1, char *dns2);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H