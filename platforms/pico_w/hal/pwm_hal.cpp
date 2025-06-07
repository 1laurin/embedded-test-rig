#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_pwm_init(uint8_t pwm_id, uint32_t frequency_hz) {
    printf("[PWM] PWM%d init at %lu Hz (stub)\n", pwm_id, frequency_hz);
    return HAL_OK;
}

hal_status_t hal_pwm_deinit(uint8_t pwm_id) {
    return HAL_OK;
}

hal_status_t hal_pwm_set_duty(uint8_t pwm_id, uint8_t channel, float duty_percent) {
    return HAL_OK;
}

hal_status_t hal_pwm_start(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}

hal_status_t hal_pwm_stop(uint8_t pwm_id, uint8_t channel) {
    return HAL_OK;
}
