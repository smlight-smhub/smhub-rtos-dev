#include "spi.h"
#include "hal_spi.h"
#include <stddef.h>

void spi_init(uint8_t spi_id, uint32_t max_speed_hz, uint8_t mode) {
    hal_spi_init(spi_id, max_speed_hz, mode);
}

void spi_transfer(uint8_t spi_id, const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len) {
    hal_spi_transfer(spi_id, tx_buf, rx_buf, len);
}
