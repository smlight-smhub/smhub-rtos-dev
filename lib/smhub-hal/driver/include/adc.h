/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#ifndef __ADC_H_
#define __ADC_H_

#include <stdint.h>

int adc_read(uint8_t channel, uint32_t *value);

#endif /* __ADC_H_ */
