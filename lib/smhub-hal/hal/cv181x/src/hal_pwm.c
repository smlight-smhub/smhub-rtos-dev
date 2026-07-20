/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "hal_pwm.h"
#include "mmio.h"
#include "hal_clock.h"
#include "smhub_arbitration.h"

#define CVI_PWM0_BASE           0x03060000

#define PWM_HLPERIOD(ch)        (0x00 + ((ch) * 8))
#define PWM_PERIOD(ch)          (0x04 + ((ch) * 8))
#define PWM_POLARITY            0x40
#define PWM_PWMSTART            0x44
#define PWM_PWMUPDATE           0x4c
#define PWM_PWM_OE              0xd0

// Count unit is 100MHz (100,000,000 counts per second), so 1 count = 10ns
#define PWM_CLK_NS              10

static uintptr_t get_pwm_base(uint8_t pwm_id) {
    if (pwm_id > 3) return 0;
    return CVI_PWM0_BASE + (pwm_id * 0x1000);
}

void hal_pwm_init(uint8_t pwm_id, uint8_t channel) {
    uintptr_t base = get_pwm_base(pwm_id);
    if (!base || channel > 3) return;

    uint32_t hw_bit = 0;
    if (pwm_id == 0) hw_bit = SMHUB_HW_PWM0;
    else if (pwm_id == 1) hw_bit = SMHUB_HW_PWM1;
    else if (pwm_id == 2) hw_bit = SMHUB_HW_PWM2;
    else if (pwm_id == 3) hw_bit = SMHUB_HW_PWM3;

    if (hw_bit && !smhub_hardware_is_released(hw_bit)) return;

    hal_clock_enable_pwm(pwm_id);
    hal_reset_deassert_pwm(pwm_id);

    // Set polarity to HIGH (default)
    uint32_t pol = mmio_read_32(base + PWM_POLARITY);
    pol |= (1 << channel);
    pol &= ~(1 << (channel + 8)); // Mode: Continuous (0)
    mmio_write_32(base + PWM_POLARITY, pol);
}

void hal_pwm_set_state(uint8_t pwm_id, uint8_t channel, uint32_t period_ns, uint32_t duty_ns, bool enable) {
    uintptr_t base = get_pwm_base(pwm_id);
    if (!base || channel > 3) return;

    uint32_t period_clk = period_ns / PWM_CLK_NS;
    uint32_t duty_clk = duty_ns / PWM_CLK_NS;

    mmio_write_32(base + PWM_HLPERIOD(channel), duty_clk);
    mmio_write_32(base + PWM_PERIOD(channel), period_clk);

    // Update registers
    uint32_t update = mmio_read_32(base + PWM_PWMUPDATE);
    update |= (1 << channel);
    mmio_write_32(base + PWM_PWMUPDATE, update);
    update &= ~(1 << channel);
    mmio_write_32(base + PWM_PWMUPDATE, update);

    uint32_t oe = mmio_read_32(base + PWM_PWM_OE);
    uint32_t start = mmio_read_32(base + PWM_PWMSTART);

    if (enable) {
        oe |= (1 << channel);
        start |= (1 << channel);
    } else {
        oe &= ~(1 << channel);
        start &= ~(1 << channel);
    }

    mmio_write_32(base + PWM_PWM_OE, oe);
    mmio_write_32(base + PWM_PWMSTART, start);
}
