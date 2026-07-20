#ifndef __HAL_PINMUX_CONFIG_H__
#define __HAL_PINMUX_CONFIG_H__
#include "cv181x_pinmux.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PIN_VOLTAGE_DOMAIN_1V8_ONLY,
    PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_1V8,
    PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_3V3,
    PIN_VOLTAGE_DOMAIN_ETH
} pin_voltage_domain_t;

void hal_pinmux_config(int io_type);
void hal_pinmux_config_raw(uint32_t pin_reg_offset, uint32_t func_val);

uint8_t hal_pad_get_drive_strength_reg(pin_voltage_domain_t domain, uint32_t microamps);
uint8_t hal_pad_get_schmitt_reg(pin_voltage_domain_t domain, uint32_t microvolts);

void hal_pad_config(uint32_t pin_reg_offset, bool pull_up, bool pull_down, uint8_t drive_strength, uint8_t schmitt, bool slew_fast, bool bus_hold);

#endif //end of __HAL_PINMUX_CONFIG_H__
