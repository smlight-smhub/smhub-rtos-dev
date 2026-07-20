/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#ifndef __HAL_DW_I2C_H_
#define __HAL_DW_I2C_H_

#include "i2c.h"

void hal_i2c_init(uint8_t i2c_id);
int hal_i2c_xfer(uint8_t i2c_id, struct i2c_msg msgs[], int num);
void hal_i2c_set_frequency(uint8_t i2c_id, uint32_t frequency);

#endif /* __HAL_DW_I2C_H_ */
