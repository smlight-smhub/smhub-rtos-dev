/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Hardware abstraction for CV181X/SG2000 PINMUX and PAD Config.
 */
//#include <stdint.h>
#include <string.h>
#include "cv181x_top_reg.h"
#include "cv181x_pinmux.h"
#include "cv181x_reg_fmux_gpio.h"
#include "cv181x_pinlist_swconfig.h"
#include "pinctrl.h"
#include "hal_pinmux.h"
#include "cv181x_pad_mapping.h"
#include "mmio.h"

void hal_pinmux_config(int io_type)
{
	switch(io_type) {
		case PINMUX_UART0:
			PINMUX_CONFIG(UART0_RX, UART0_RX);
			PINMUX_CONFIG(UART0_TX, UART0_TX);
		break;
		case PINMUX_I2C0:
			PINMUX_CONFIG(IIC0_SCL, IIC0_SCL);
			PINMUX_CONFIG(IIC0_SDA, IIC0_SDA);
		break;
		case PINMUX_I2C2:
			// Often mapped to PAD_MIPI_TXM3 / TXP3 on smhub-6
			PINMUX_CONFIG(PAD_MIPI_TXM3, IIC2_SDA);
			PINMUX_CONFIG(PAD_MIPI_TXP3, IIC2_SCL);
		break;
		case PINMUX_I2C3:
			PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
			PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
		break;
		case PINMUX_I2C4:
			// Assuming VIVO_D1 and VIVO_D0 based on standard SG2000 mapping
			PINMUX_CONFIG(VIVO_D1, IIC4_SCL);
			PINMUX_CONFIG(VIVO_D0, IIC4_SDA);
		break;
		case PINMUX_CAM0:
			PINMUX_CONFIG(CAM_RST0, XGPIOA_2);
			PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);

		break;
		case PINMUX_CAM1:
			// PINMUX_CONFIG(CAM_RST0, XGPIOA_2);
			PINMUX_CONFIG(CAM_MCLK1, CAM_MCLK1);
		break;
		case PINMUX_SPI0:
			PINMUX_CONFIG(PAD_MIPI_TXM1, SPI0_SDO);
		break;
		case 100: // Custom PINMUX_ADC1
			hal_pinmux_config_raw(FMUX_GPIO_FUNCSEL_ADC1, 0);
		break;
		case 101: // Custom PINMUX_ADC2
			hal_pinmux_config_raw(FMUX_GPIO_FUNCSEL_ADC2, 0);
		break;
		case 102: // Custom PINMUX_ADC3
			hal_pinmux_config_raw(FMUX_GPIO_FUNCSEL_ADC3, 0);
		break;
		default:
			break;
	}
}

void hal_pinmux_config_raw(uint32_t pin_reg_offset, uint32_t func_val) {
    mmio_clrsetbits_32(PINMUX_BASE + pin_reg_offset, 0x7, func_val);
}

static const uint32_t cv1800b_1v8_oc_map[] = {
	12800, 25300, 37400, 49000
};

static const uint32_t cv1800b_18od33_1v8_oc_map[] = {
	7800, 11700, 15500, 19200, 23000, 26600, 30200, 33700
};

static const uint32_t cv1800b_18od33_3v3_oc_map[] = {
	5500, 8200, 10800, 13400, 16100, 18700, 21200, 23700
};

static const uint32_t cv1800b_eth_oc_map[] = {
	15700, 17800
};

uint8_t hal_pad_get_drive_strength_reg(pin_voltage_domain_t domain, uint32_t microamps) {
    const uint32_t *map = NULL;
    uint8_t count = 0;

    switch (domain) {
        case PIN_VOLTAGE_DOMAIN_1V8_ONLY:
            map = cv1800b_1v8_oc_map;
            count = sizeof(cv1800b_1v8_oc_map) / sizeof(uint32_t);
            break;
        case PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_1V8:
            map = cv1800b_18od33_1v8_oc_map;
            count = sizeof(cv1800b_18od33_1v8_oc_map) / sizeof(uint32_t);
            break;
        case PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_3V3:
            map = cv1800b_18od33_3v3_oc_map;
            count = sizeof(cv1800b_18od33_3v3_oc_map) / sizeof(uint32_t);
            break;
        case PIN_VOLTAGE_DOMAIN_ETH:
            map = cv1800b_eth_oc_map;
            count = sizeof(cv1800b_eth_oc_map) / sizeof(uint32_t);
            break;
        default:
            return 0;
    }

    uint8_t best_idx = 0;
    uint32_t min_diff = 0xFFFFFFFF;

    for (uint8_t i = 0; i < count; i++) {
        uint32_t diff = (microamps > map[i]) ? (microamps - map[i]) : (map[i] - microamps);
        if (diff < min_diff) {
            min_diff = diff;
            best_idx = i;
        }
    }

    return best_idx;
}


static const uint32_t cv1800b_1v8_schmitt_map[] = { 0, 970000, 1040000 };
static const uint32_t cv1800b_18od33_1v8_schmitt_map[] = { 0, 1070000 };
static const uint32_t cv1800b_18od33_3v3_schmitt_map[] = { 0, 1100000 };

uint8_t hal_pad_get_schmitt_reg(pin_voltage_domain_t domain, uint32_t microvolts) {
    const uint32_t *map = NULL;
    uint8_t count = 0;

    switch (domain) {
        case PIN_VOLTAGE_DOMAIN_1V8_ONLY:
            map = cv1800b_1v8_schmitt_map;
            count = sizeof(cv1800b_1v8_schmitt_map) / sizeof(uint32_t);
            break;
        case PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_1V8:
            map = cv1800b_18od33_1v8_schmitt_map;
            count = sizeof(cv1800b_18od33_1v8_schmitt_map) / sizeof(uint32_t);
            break;
        case PIN_VOLTAGE_DOMAIN_1V8_OR_3V3_AT_3V3:
            map = cv1800b_18od33_3v3_schmitt_map;
            count = sizeof(cv1800b_18od33_3v3_schmitt_map) / sizeof(uint32_t);
            break;
        default:
            return 0;
    }

    uint8_t best_idx = 0;
    uint32_t min_diff = 0xFFFFFFFF;

    for (uint8_t i = 0; i < count; i++) {
        uint32_t diff = (microvolts > map[i]) ? (microvolts - map[i]) : (map[i] - microvolts);
        if (diff < min_diff) {
            min_diff = diff;
            best_idx = i;
        }
    }

    return best_idx;
}

void hal_pad_config(uint32_t pin_reg_offset, bool pull_up, bool pull_down, uint8_t drive_strength, uint8_t schmitt, bool slew_fast, bool bus_hold) {
    uint32_t pad_reg_offset = hal_get_pad_offset(pin_reg_offset);
    if (pad_reg_offset == 0xFFFFFFFF) return;

    // CV181x Pad Control Register Layout (from pinctrl-cv18xx.h):
    // Bit 2: Pull-up (PU)
    // Bit 3: Pull-down (PD)
    // Bits 5-7: Drive strength (DS)
    // Bits 8-9: Schmitt Trigger
    // Bit 10: Bus Hold
    // Bit 11: Fast Slew Rate
    uint32_t mask = (1 << 2) | (1 << 3) | (0x7 << 5) | (0x3 << 8) | (1 << 10) | (1 << 11);
    uint32_t val = 0;
    
    if (pull_up) val |= (1 << 2);
    if (pull_down) val |= (1 << 3);
    val |= ((drive_strength & 0x7) << 5);
    val |= ((schmitt & 0x3) << 8);
    if (bus_hold) val |= (1 << 10);
    if (slew_fast) val |= (1 << 11);
    
    mmio_clrsetbits_32(PINMUX_BASE + pad_reg_offset, mask, val);
}
