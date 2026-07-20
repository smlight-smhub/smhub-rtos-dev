#ifndef __HAL_SPI_H__
#define __HAL_SPI_H__
#include <stdint.h>
#include <stdbool.h>

void hal_spi_init(uint8_t spi_id, uint32_t max_speed_hz, uint8_t mode);
void hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);

#endif
