/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#ifndef __I2C_H_
#define __I2C_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct i2c_msg {
    uint16_t addr;      /* slave address */
    uint16_t flags;
#define I2C_M_TEN           0x0010  /* ten bit chip address */
#define I2C_M_RD            0x0001  /* read data */
#define I2C_M_WR            0x0000  /* write data */
#define I2C_M_STOP          0x8000
    uint16_t len;       /* msg length */
    uint8_t *buf;       /* pointer to msg data */
};

void i2c_init(uint8_t i2c_id);
int i2c_xfer(uint8_t i2c_id, struct i2c_msg msgs[], int num);
int i2c_write(uint8_t i2c_id, uint8_t dev, uint16_t addr, uint16_t alen, uint8_t *buffer, uint16_t len);
int i2c_read(uint8_t i2c_id, uint8_t dev, uint16_t addr, uint16_t alen, uint8_t *buffer, uint16_t len);
void i2c_set_frequency(uint8_t i2c_id, uint32_t frequency);

#endif /* __I2C_H_ */
