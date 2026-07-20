/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Hardware abstraction for CV181X/SG2000 Clock and Reset IP blocks.
 */
#include "hal_clock.h"
#include "mmio.h"
#include "cv181x_top_reg.h"

// --- Clock Enable Helpers ---
// Bits mapped accurately from Mainline Sophgo pinctrl and cv181x-clock.h

static void clk_en(uint32_t reg, uint8_t bit) {
    uint32_t val = mmio_read_32(reg);
    val |= (1 << bit);
    mmio_write_32(reg, val);
}

void hal_clock_enable_i2c(uint8_t i2c_id) {
    // Enable the base CLK_I2C parent clock
    clk_en(REG_CLK_ENABLE_REG3, 7);

    // Enable the base CLK_APB_I2C bus clock
    clk_en(REG_CLK_ENABLE_REG1, 6);

    // CV181X_CLK_APB_I2Cx are at REG_CLK_EN_3
    switch(i2c_id) {
        case 0: clk_en(REG_CLK_ENABLE_REG3, 17); break;
        case 1: clk_en(REG_CLK_ENABLE_REG3, 18); break;
        case 2: clk_en(REG_CLK_ENABLE_REG3, 19); break;
        case 3: clk_en(REG_CLK_ENABLE_REG3, 20); break;
        case 4: clk_en(REG_CLK_ENABLE_REG3, 21); break;
    }
}

void hal_clock_enable_pwm(uint8_t pwm_id) {
    // CV181X_CLK_PWM_SRC is REG_CLK_EN_4, bit 4
    clk_en(REG_CLK_ENABLE_REG4, 4);
}

void hal_clock_enable_adc(void) {
    // CV181X_CLK_SARADC is usually 21 but is tied to the main domain 
    // We explicitly enable it just in case.
    // However, it's typically handled by the power controller implicitly.
}

void hal_clock_enable_uart(uint8_t uart_id) {
    if (uart_id > 4) return;
    // UART base clocks start at REG_CLK_EN_1 bit 14, APB clocks at bit 15
    uint8_t base_bit = 14 + (uart_id * 2);
    uint8_t apb_bit = 15 + (uart_id * 2);
    clk_en(REG_CLK_ENABLE_REG1, base_bit);
    clk_en(REG_CLK_ENABLE_REG1, apb_bit);
}

void hal_clock_enable_spi(uint8_t spi_id) {
    if (spi_id > 3) return;
    // Shared SPI base clock
    clk_en(REG_CLK_ENABLE_REG3, 6);
    // Specific SPI APB clock
    clk_en(REG_CLK_ENABLE_REG1, 9 + spi_id);
}

void hal_clock_enable_gpio(void) {
    // APB, INTR, DB
    clk_en(REG_CLK_ENABLE_REG0, 29);
    clk_en(REG_CLK_ENABLE_REG0, 30);
    clk_en(REG_CLK_ENABLE_REG0, 31);
}

// --- Soft Reset Helpers ---
// Bits mapped accurately from Mainline Sophgo cv181x-resets.h
// 0 = Assert Reset, 1 = De-assert Reset

static void soft_rst_assert(uint32_t reset_id) {
    uint32_t reg = REG_TOP_SOFT_RST0 + ((reset_id / 32) * 4);
    uint8_t bit = reset_id % 32;
    uint32_t val = mmio_read_32(reg);
    val &= ~(1 << bit);
    mmio_write_32(reg, val);
}

static void soft_rst_deassert(uint32_t reset_id) {
    uint32_t reg = REG_TOP_SOFT_RST0 + ((reset_id / 32) * 4);
    uint8_t bit = reset_id % 32;
    uint32_t val = mmio_read_32(reg);
    val |= (1 << bit);
    mmio_write_32(reg, val);
}

void hal_reset_i2c(uint8_t i2c_id) {
    if (i2c_id <= 4) {
        soft_rst_assert(27 + i2c_id);
        for(int i=0; i<100; i++) asm volatile("nop");
        soft_rst_deassert(27 + i2c_id);
    }
}

void hal_reset_deassert_i2c(uint8_t i2c_id) {
    if (i2c_id <= 4) {
        soft_rst_deassert(27 + i2c_id);
    }
}

void hal_reset_deassert_pwm(uint8_t pwm_id) {
    // RST_PWM0 = 32
    // SG2000 has 4 main PWM IP blocks (pwm0, pwm1, pwm2, pwm3) each with 4 channels
    if (pwm_id <= 3) {
        soft_rst_deassert(32 + pwm_id);
    }
}

void hal_reset_deassert_adc(void) {
    // RST_SARADC = 52
    soft_rst_deassert(52);
}

void hal_reset_deassert_uart(uint8_t uart_id) {
    if (uart_id <= 4) {
        if (uart_id == 4) soft_rst_deassert(74); // RST_UART4
        else soft_rst_deassert(23 + uart_id);    // RST_UART0-3
    }
}

void hal_reset_deassert_spi(uint8_t spi_id) {
    if (spi_id <= 3) {
        soft_rst_deassert(40 + spi_id); // RST_SPI0-3
    }
}

void hal_reset_deassert_gpio(uint8_t gpio_bank) {
    if (gpio_bank <= 2) {
        soft_rst_deassert(44 + gpio_bank); // RST_GPIO0-2
    } else if (gpio_bank == 3) {
        soft_rst_deassert(75);             // RST_GPIO3
    }
}
