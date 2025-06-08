/**
 * @file wifi_manager.h
 * @brief Simplified WiFi manager header for Raspberry Pi Pico W
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // =============================================================================
    // CONSTANTS
    // =============================================================================

#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64

    // =============================================================================
    // PUBLIC FUNCTION DECLARATIONS
    // =============================================================================

    /**
     * @brief Initialize the WiFi manager
     * @return true if initialization was successful, false otherwise
     */
    bool wifi_manager_init(void);

    /**
     * @brief Deinitialize the WiFi manager
     */
    void wifi_manager_deinit(void);

    /**
     * @brief Connect to a WiFi network
     * @param ssid The network SSID
     * @param password The network password (can be NULL for open networks)
     * @return true if connection was successful, false otherwise
     */
    bool wifi_connect(const char *ssid, const char *password);

    /**
     * @brief Disconnect from the current WiFi network
     */
    void wifi_disconnect(void);

    /**
     * @brief Check if WiFi is currently connected
     * @return true if connected, false otherwise
     */
    bool wifi_is_connected(void);

    /**
     * @brief Get the current IP address
     * @return The IP address as a string, or "0.0.0.0" if not connected
     */
    const char *wifi_get_ip_address(void);

    /**
     * @brief Get the current SSID
     * @return The SSID as a string
     */
    const char *wifi_get_ssid(void);

    /**
     * @brief Get the current signal strength
     * @return The RSSI value in dBm
     */
    int wifi_get_rssi(void);

    /**
     * @brief Update the WiFi manager (call this periodically)
     */
    void wifi_manager_update(void);

    /**
     * @brief Set the WiFi hostname
     * @param hostname The hostname to set
     */
    void wifi_set_hostname(const char *hostname);

    // =============================================================================
    // HELPER FUNCTIONS
    // =============================================================================

    /**
     * @brief Helper function for board_config.h macros
     */
    bool wifi_is_connected_helper(void);

    /**
     * @brief Set the WiFi LED state
     * @param state true to turn on, false to turn off
     */
    void wifi_set_led(bool state);

    /**
     * @brief Toggle the WiFi LED state
     */
    void wifi_toggle_led(void);

    // =============================================================================
    // COMPATIBILITY FUNCTIONS (simplified implementations)
    // =============================================================================

    /**
     * @brief Register an event callback (simplified - does nothing in this version)
     * @param callback The callback function pointer
     */
    void wifi_register_event_callback(void *callback);

    /**
     * @brief Scan for available networks (simplified - does nothing in this version)
     * @return false (not implemented)
     */
    bool wifi_scan_networks(void);

    /**
     * @brief Register a scan callback (simplified - does nothing in this version)
     * @param callback The callback function pointer
     */
    void wifi_register_scan_callback(void *callback);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H