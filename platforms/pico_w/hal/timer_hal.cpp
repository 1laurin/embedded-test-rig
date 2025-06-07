/**
 * @file timer_hal.cpp
 * @brief Timer HAL stub for Pico W
 */

#include "hal_interface.h"
#include <stdio.h>

hal_status_t hal_timer_init(uint8_t timer_id, const timer_config_t *config) {
    printf("[TIMER] Timer%d init stub\n", timer_id);
    return HAL_OK;
}

hal_status_t hal_timer_deinit(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_start(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_stop(uint8_t timer_id) {
    return HAL_OK;
}

hal_status_t hal_timer_get_count(uint8_t timer_id, uint32_t *count) {
    if (count) *count = 0;
    return HAL_OK;
}

hal_status_t hal_timer_reset(uint8_t timer_id) {
    return HAL_OK;
}
