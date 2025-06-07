#include "../../platforms/common/hal_interface.h"
#include <stdio.h>

hal_status_t hal_spi_init(uint8_t spi_id, const spi_config_t *config) {
    printf("[SPI] SPI%d init (stub)\n", spi_id);
    return HAL_OK;
}

hal_status_t hal_spi_deinit(uint8_t spi_id) {
    return HAL_OK;
}

hal_status_t hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_data, uint8_t *rx_data, size_t size, uint32_t timeout_ms) {
    return HAL_NOT_SUPPORTED;
}

hal_status_t hal_spi_set_cs(uint8_t spi_id, uint32_t cs_pin, bool active) {
    return HAL_NOT_SUPPORTED;
}
