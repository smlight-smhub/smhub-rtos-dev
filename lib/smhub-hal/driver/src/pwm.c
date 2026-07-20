/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "pwm.h"
#include "hal_pwm.h"

void pwm_init_channel(uint8_t pwm_id, uint8_t channel) {
    hal_pwm_init(pwm_id, channel);
}

void pwm_set_state(uint8_t pwm_id, uint8_t channel, uint32_t period_ns, uint32_t duty_ns, bool enable) {
    hal_pwm_set_state(pwm_id, channel, period_ns, duty_ns, enable);
}
