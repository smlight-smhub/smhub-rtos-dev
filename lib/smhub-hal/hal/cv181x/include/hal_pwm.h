/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#ifndef __HAL_PWM_H_
#define __HAL_PWM_H_

#include <stdint.h>
#include <stdbool.h>

void hal_pwm_init(uint8_t pwm_id, uint8_t channel);
void hal_pwm_set_state(uint8_t pwm_id, uint8_t channel, uint32_t period_ns, uint32_t duty_ns, bool enable);

#endif /* __HAL_PWM_H_ */
