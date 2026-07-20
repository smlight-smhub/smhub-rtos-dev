/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 */
#ifndef __HAL_CLOCK_H__
#define __HAL_CLOCK_H__

#include <stdint.h>
#include <stdbool.h>

void hal_clock_enable_i2c(uint8_t i2c_id);
void hal_clock_enable_pwm(uint8_t pwm_id);
void hal_clock_enable_adc(void);
void hal_clock_enable_uart(uint8_t uart_id);
void hal_clock_enable_spi(uint8_t spi_id);
void hal_clock_enable_gpio(void);

void hal_reset_i2c(uint8_t i2c_id);
void hal_reset_deassert_i2c(uint8_t i2c_id);
void hal_reset_deassert_pwm(uint8_t pwm_id);
void hal_reset_deassert_adc(void);
void hal_reset_deassert_uart(uint8_t uart_id);
void hal_reset_deassert_spi(uint8_t spi_id);
void hal_reset_deassert_gpio(uint8_t gpio_bank);

#endif // __HAL_CLOCK_H__
