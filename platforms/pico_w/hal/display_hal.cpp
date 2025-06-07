/**
 * @file display_hal.cpp
 * @brief Display Hardware Abstraction Layer implementation for Raspberry Pi Pico W
 *
 * This file implements the Display HAL interface for the Pico W platform using
 * a console-based display simulation (simplified for build compatibility).
 *
 * @author Multi-Channel Diagnostic Test Rig Team
 * @date 2025
 */

#include "../../platforms/common/hal_interface.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// =============================================================================
// PRIVATE CONSTANTS
// =============================================================================

#define DISPLAY_COMMAND_BUFFER_SIZE 256
#define MAX_TEXT_LENGTH 64

// =============================================================================
// PRIVATE TYPES
// =============================================================================

typedef struct
{
    bool initialized;
    uint16_t width;
    uint16_t height;
    uint8_t brightness;
    uint32_t bg_color;
    uint32_t last_pixel_x;
    uint32_t last_pixel_y;
    uint32_t last_pixel_color;
} display_context_t;

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

static display_context_t display_ctx = {0};

// =============================================================================
// PRIVATE FUNCTION DECLARATIONS
// =============================================================================

static uint32_t rgb888_to_rgb565(uint32_t rgb888);
static void log_display_operation(const char *operation, const char *details);

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize display subsystem
 * @return HAL status code
 */
hal_status_t hal_display_init(void)
{
    if (display_ctx.initialized)
    {
        return HAL_OK; // Already initialized
    }

    printf("[DISPLAY] Initializing display subsystem...\n");

    // Initialize display context
    display_ctx.width = DISPLAY_WIDTH;
    display_ctx.height = DISPLAY_HEIGHT;
    display_ctx.brightness = 100;  // Full brightness
    display_ctx.bg_color = 0x0000; // Black background
    display_ctx.last_pixel_x = 0;
    display_ctx.last_pixel_y = 0;
    display_ctx.last_pixel_color = 0;

    display_ctx.initialized = true;

    printf("[DISPLAY] Console-based display initialized (%dx%d)\n",
           display_ctx.width, display_ctx.height);
    printf("[DISPLAY] Display simulation active - commands will be logged\n");

    return HAL_OK;
}

/**
 * @brief Deinitialize display subsystem
 * @return HAL status code
 */
hal_status_t hal_display_deinit(void)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    printf("[DISPLAY] Deinitializing display...\n");

    // Clear display state
    memset(&display_ctx, 0, sizeof(display_context_t));

    printf("[DISPLAY] Display deinitialized\n");

    return HAL_OK;
}

/**
 * @brief Clear display
 * @param color Clear color (RGB888 format)
 * @return HAL status code
 */
hal_status_t hal_display_clear(uint32_t color)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    uint32_t rgb565_color = rgb888_to_rgb565(color);
    display_ctx.bg_color = rgb565_color;

    char details[64];
    snprintf(details, sizeof(details), "color=0x%06lX (RGB565: 0x%04lX)",
             color, rgb565_color);
    log_display_operation("CLEAR", details);

    return HAL_OK;
}

/**
 * @brief Update display with buffer data
 * @param buffer Display buffer structure
 * @return HAL status code
 */
hal_status_t hal_display_update(const display_buffer_t *buffer)
{
    if (!display_ctx.initialized || buffer == nullptr)
    {
        return HAL_INVALID_PARAM;
    }

    // Validate buffer dimensions
    if (buffer->x_offset + buffer->width > display_ctx.width ||
        buffer->y_offset + buffer->height > display_ctx.height)
    {
        return HAL_INVALID_PARAM;
    }

    char details[128];
    snprintf(details, sizeof(details),
             "pos=(%d,%d) size=%dx%d bytes=%zu",
             buffer->x_offset, buffer->y_offset,
             buffer->width, buffer->height,
             buffer->data_size);
    log_display_operation("UPDATE", details);

    return HAL_OK;
}

/**
 * @brief Set display pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color (RGB888 format)
 * @return HAL status code
 */
hal_status_t hal_display_set_pixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    if (x >= display_ctx.width || y >= display_ctx.height)
    {
        return HAL_INVALID_PARAM;
    }

    uint32_t rgb565_color = rgb888_to_rgb565(color);

    // Store last pixel operation
    display_ctx.last_pixel_x = x;
    display_ctx.last_pixel_y = y;
    display_ctx.last_pixel_color = rgb565_color;

    // Only log occasional pixels to avoid spam
    static uint32_t pixel_count = 0;
    pixel_count++;

    if (pixel_count % 100 == 1)
    { // Log every 100th pixel
        char details[64];
        snprintf(details, sizeof(details), "pos=(%d,%d) color=0x%06lX", x, y, color);
        log_display_operation("PIXEL", details);
    }

    return HAL_OK;
}

/**
 * @brief Draw rectangle on display
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Rectangle color (RGB888 format)
 * @param filled Fill rectangle if true
 * @return HAL status code
 */
hal_status_t hal_display_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, bool filled)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    if (x >= display_ctx.width || y >= display_ctx.height ||
        x + width > display_ctx.width || y + height > display_ctx.height)
    {
        return HAL_INVALID_PARAM;
    }

    uint32_t rgb565_color = rgb888_to_rgb565(color);

    char details[128];
    snprintf(details, sizeof(details),
             "pos=(%d,%d) size=%dx%d color=0x%06lX %s",
             x, y, width, height, color, filled ? "filled" : "outline");
    log_display_operation("RECT", details);

    return HAL_OK;
}

/**
 * @brief Draw text on display
 * @param x X coordinate
 * @param y Y coordinate
 * @param text Text string
 * @param color Text color (RGB888 format)
 * @param bg_color Background color (RGB888 format)
 * @return HAL status code
 */
hal_status_t hal_display_draw_text(uint16_t x, uint16_t y, const char *text, uint32_t color, uint32_t bg_color)
{
    if (!display_ctx.initialized || text == nullptr)
    {
        return HAL_INVALID_PARAM;
    }

    if (x >= display_ctx.width || y >= display_ctx.height)
    {
        return HAL_INVALID_PARAM;
    }

    // Truncate text if too long
    char safe_text[MAX_TEXT_LENGTH];
    strncpy(safe_text, text, sizeof(safe_text) - 1);
    safe_text[sizeof(safe_text) - 1] = '\0';

    uint32_t rgb565_color = rgb888_to_rgb565(color);
    uint32_t rgb565_bg = rgb888_to_rgb565(bg_color);

    char details[256];
    snprintf(details, sizeof(details),
             "pos=(%d,%d) text=\"%s\" color=0x%06lX bg=0x%06lX",
             x, y, safe_text, color, bg_color);
    log_display_operation("TEXT", details);

    return HAL_OK;
}

/**
 * @brief Set display brightness
 * @param brightness Brightness level (0-100)
 * @return HAL status code
 */
hal_status_t hal_display_set_brightness(uint8_t brightness)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    if (brightness > 100)
    {
        return HAL_INVALID_PARAM;
    }

    display_ctx.brightness = brightness;

    char details[32];
    snprintf(details, sizeof(details), "level=%d%%", brightness);
    log_display_operation("BRIGHTNESS", details);

    return HAL_OK;
}

/**
 * @brief Flush display buffer to screen
 * @return HAL status code
 */
hal_status_t hal_display_flush(void)
{
    if (!display_ctx.initialized)
    {
        return HAL_ERROR;
    }

    log_display_operation("FLUSH", "rendering complete");

    return HAL_OK;
}

// =============================================================================
// PRIVATE FUNCTIONS
// =============================================================================

/**
 * @brief Convert RGB888 to RGB565 format
 * @param rgb888 24-bit RGB color
 * @return 16-bit RGB565 color
 */
static uint32_t rgb888_to_rgb565(uint32_t rgb888)
{
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;

    // Convert to 5-6-5 format
    uint16_t r565 = (r >> 3) & 0x1F;
    uint16_t g565 = (g >> 2) & 0x3F;
    uint16_t b565 = (b >> 3) & 0x1F;

    return (r565 << 11) | (g565 << 5) | b565;
}

/**
 * @brief Log display operation for debugging
 * @param operation Operation name
 * @param details Operation details
 */
static void log_display_operation(const char *operation, const char *details)
{
    printf("[DISPLAY] %s: %s\n", operation, details);
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Get display dimensions
 * @param width Pointer to store width
 * @param height Pointer to store height
 */
void pico_display_get_dimensions(uint16_t *width, uint16_t *height)
{
    if (width)
        *width = display_ctx.width;
    if (height)
        *height = display_ctx.height;
}

/**
 * @brief Get current display brightness
 * @return Brightness level (0-100)
 */
uint8_t pico_display_get_brightness(void)
{
    return display_ctx.brightness;
}

/**
 * @brief Check if display is ready for commands
 * @return true if ready, false otherwise
 */
bool pico_display_is_ready(void)
{
    return display_ctx.initialized;
}

/**
 * @brief Draw a simple progress bar
 * @param x X position
 * @param y Y position
 * @param width Bar width
 * @param height Bar height
 * @param progress Progress percentage (0-100)
 * @param fg_color Foreground color
 * @param bg_color Background color
 */
void pico_display_draw_progress_bar(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                    uint8_t progress, uint32_t fg_color, uint32_t bg_color)
{
    if (!display_ctx.initialized || progress > 100)
    {
        return;
    }

    // Draw background
    hal_display_draw_rect(x, y, width, height, bg_color, true);

    // Draw progress
    uint16_t progress_width = (width * progress) / 100;
    if (progress_width > 0)
    {
        hal_display_draw_rect(x, y, progress_width, height, fg_color, true);
    }

    // Draw border
    hal_display_draw_rect(x, y, width, height, 0xFFFFFF, false);

    char details[128];
    snprintf(details, sizeof(details),
             "pos=(%d,%d) size=%dx%d progress=%d%% fg=0x%06lX bg=0x%06lX",
             x, y, width, height, progress, fg_color, bg_color);
    log_display_operation("PROGRESS", details);
}

/**
 * @brief Display system status on screen
 * @param uptime_ms System uptime in milliseconds
 * @param loop_count Main loop iteration count
 */
void pico_display_show_status(uint32_t uptime_ms, uint32_t loop_count)
{
    if (!display_ctx.initialized)
    {
        return;
    }

    // Clear display
    hal_display_clear(0x000080); // Dark blue background

    // Title
    hal_display_draw_text(10, 10, "Pico W Diagnostic Rig", 0xFFFFFF, 0x000080);

    // Status information
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Uptime: %lu s", uptime_ms / 1000);
    hal_display_draw_text(10, 30, status_text, 0x00FF00, 0x000080);

    snprintf(status_text, sizeof(status_text), "Loop: %lu", loop_count);
    hal_display_draw_text(10, 50, status_text, 0x00FF00, 0x000080);

    // System health indicator
    hal_display_draw_text(10, 80, "Status:", 0xFFFF00, 0x000080);
    hal_display_draw_text(80, 80, "RUNNING", 0x00FF00, 0x000080);

    // Progress bar showing system health
    pico_display_draw_progress_bar(10, 100, 200, 20, 95, 0x00FF00, 0x333333);

    hal_display_flush();
}