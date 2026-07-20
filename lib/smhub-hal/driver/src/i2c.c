/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "i2c.h"
#include "hal_dw_i2c.h"

void i2c_init(uint8_t i2c_id)
{
    hal_i2c_init(i2c_id);
}

int i2c_xfer(uint8_t i2c_id, struct i2c_msg msgs[], int num)
{
    return hal_i2c_xfer(i2c_id, msgs, num);
}

int i2c_write(uint8_t i2c_id, uint8_t dev, uint16_t addr, uint16_t alen, uint8_t *buffer, uint16_t len)
{
    struct i2c_msg msg;
    uint8_t buf[256];
    
    // Convert register addr + buffer into a single payload if alen > 0
    int ptr = 0;
    while (alen) {
        alen--;
        buf[ptr++] = (addr >> (alen * 8)) & 0xff;
    }
    for (int i = 0; i < len && ptr < sizeof(buf); i++) {
        buf[ptr++] = buffer[i];
    }
    
    msg.addr = dev;
    msg.flags = I2C_M_WR;
    msg.len = ptr;
    msg.buf = buf;

    return hal_i2c_xfer(i2c_id, &msg, 1);
}

int i2c_read(uint8_t i2c_id, uint8_t dev, uint16_t addr, uint16_t alen, uint8_t *buffer, uint16_t len)
{
    struct i2c_msg msgs[2];
    uint8_t addr_buf[4];
    int num_msgs = 0;

    if (alen > 0) {
        int ptr = 0;
        while (alen && ptr < sizeof(addr_buf)) {
            alen--;
            addr_buf[ptr++] = (addr >> (alen * 8)) & 0xff;
        }
        msgs[num_msgs].addr = dev;
        msgs[num_msgs].flags = I2C_M_WR;
        msgs[num_msgs].len = ptr;
        msgs[num_msgs].buf = addr_buf;
        num_msgs++;
    }

    if (len > 0) {
        msgs[num_msgs].addr = dev;
        msgs[num_msgs].flags = I2C_M_RD;
        msgs[num_msgs].len = len;
        msgs[num_msgs].buf = buffer;
        num_msgs++;
    }

    if (num_msgs == 0) return 0;
    return hal_i2c_xfer(i2c_id, msgs, num_msgs);
}

void i2c_set_frequency(uint8_t i2c_id, uint32_t frequency)
{
    hal_i2c_set_frequency(i2c_id, frequency);
}
