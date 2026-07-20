/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "adc.h"
#include "hal_adc.h"

int adc_read(uint8_t channel, uint32_t *value) {
    return hal_adc_read(channel, value);
}
