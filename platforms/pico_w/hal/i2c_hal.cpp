#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_i2c_init(uint8_t i2c_id, const i2c_config_t *config) {
    printf("[I2C] I2C%d init (stub)\n", i2c_id);
    return HAL_OK;
}

hal_status_t hal_i2c_deinit(uint8_t i2c_id) {
    return HAL_OK;
}

hal_status_t hal_i2c_transmit(uint8_t i2c_id, uint8_t device_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_receive(uint8_t i2c_id, uint8_t device_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_write_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_i2c_read_register(uint8_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}
