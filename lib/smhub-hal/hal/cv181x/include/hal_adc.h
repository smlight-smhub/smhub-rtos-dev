/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#ifndef __HAL_ADC_H_
#define __HAL_ADC_H_

#include <stdint.h>

int hal_adc_read(uint8_t channel, uint32_t *value);

#endif /* __HAL_ADC_H_ */
