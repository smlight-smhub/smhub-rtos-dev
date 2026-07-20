#ifndef _SPI_H_
#define _SPI_H_
#include <stdint.h>

void spi_init(uint8_t spi_id, uint32_t max_speed_hz, uint8_t mode);
void spi_transfer(uint8_t spi_id, const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);

#endif
