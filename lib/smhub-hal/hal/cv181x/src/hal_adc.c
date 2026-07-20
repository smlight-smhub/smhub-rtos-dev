/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "hal_adc.h"
#include "mmio.h"
#include "hal_clock.h"
#include "FreeRTOS.h"
#include "task.h"
#include "smhub_arbitration.h"

#define SARADC_BASE                 0x030F0000
#define SARADC_CH_MAX               3

#define SARADC_CTRL_OFFSET          0x04
#define SARADC_CTRL_START           (1 << 0)
#define SARADC_CTRL_SEL_POS         0x04

#define SARADC_STATUS_OFFSET        0x08
#define SARADC_STATUS_BUSY          (1 << 0)

#define SARADC_CYC_SET_OFFSET       0x0C
#define SARADC_CYC_CLKDIV_DIV_16    (15U << 12)

#define SARADC_RESULT_OFFSET        0x14
#define SARADC_RESULT(n)            (SARADC_RESULT_OFFSET + ((n) - 1) * 4)

#define SARADC_RESULT_MASK          0x0FFF
#define SARADC_RESULT_VALID         (1 << 15)

static void cvi_set_saradc_ctrl(uint32_t value) {
    value |= mmio_read_32(SARADC_BASE + SARADC_CTRL_OFFSET);
    mmio_write_32(SARADC_BASE + SARADC_CTRL_OFFSET, value);
}

static void cvi_reset_saradc_ctrl(uint32_t value) {
    uint32_t current = mmio_read_32(SARADC_BASE + SARADC_CTRL_OFFSET);
    current &= ~value;
    mmio_write_32(SARADC_BASE + SARADC_CTRL_OFFSET, current);
}

int hal_adc_read(uint8_t channel, uint32_t *value) {
    if (channel > SARADC_CH_MAX || channel == 0) {
        return -1;
    }

    uint32_t hw_bit = 0;
    if (channel == 1) hw_bit = SMHUB_HW_ADC1;
    else if (channel == 2) hw_bit = SMHUB_HW_ADC2;

    if (hw_bit && !smhub_hardware_is_released(hw_bit)) return -1;

    hal_clock_enable_adc();
    hal_reset_deassert_adc();

    // Enable ADC channel
    cvi_set_saradc_ctrl(1 << (SARADC_CTRL_SEL_POS + channel));
    
    // Set cyc (clock div)
    uint32_t cyc = mmio_read_32(SARADC_BASE + SARADC_CYC_SET_OFFSET);
    cyc &= ~SARADC_CYC_CLKDIV_DIV_16;
    mmio_write_32(SARADC_BASE + SARADC_CYC_SET_OFFSET, cyc);
    cyc |= SARADC_CYC_CLKDIV_DIV_16;
    mmio_write_32(SARADC_BASE + SARADC_CYC_SET_OFFSET, cyc);

    // Start conversion
    cvi_set_saradc_ctrl(SARADC_CTRL_START);

    // Wait for conversion
    int timeout = 1000;
    while (mmio_read_32(SARADC_BASE + SARADC_STATUS_OFFSET) & SARADC_STATUS_BUSY) {
        if (--timeout == 0) return -2;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    uint32_t result = mmio_read_32(SARADC_BASE + SARADC_RESULT(channel));
    
    // Disable ADC
    cvi_reset_saradc_ctrl(1 << (SARADC_CTRL_SEL_POS + channel));

    if (result & SARADC_RESULT_VALID) {
        *value = result & SARADC_RESULT_MASK;
        return 0;
    }

    return -3;
}
