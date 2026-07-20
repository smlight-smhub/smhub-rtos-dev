#include "hal_spi.h"
#include "cv181x_top_reg.h"
#include "hal_clock.h"
#include <stdio.h>

// Assuming 100MHz for SPI input clock on CV181X
#define SPI_INPUT_CLK 100000000

struct dw_spi_regs {
    volatile uint32_t ctrlr0;
    volatile uint32_t ctrlr1;
    volatile uint32_t ssienr;
    volatile uint32_t mwcr;
    volatile uint32_t ser;
    volatile uint32_t baudr;
    volatile uint32_t txftlr;
    volatile uint32_t rxftlr;
    volatile uint32_t txflr;
    volatile uint32_t rxflr;
    volatile uint32_t sr;
    volatile uint32_t imr;
    volatile uint32_t isr;
    volatile uint32_t risr;
    volatile uint32_t txoicr;
    volatile uint32_t rxoicr;
    volatile uint32_t rxuicr;
    volatile uint32_t msticr;
    volatile uint32_t icr;
    volatile uint32_t dmacr;
    volatile uint32_t dmatdlr;
    volatile uint32_t dmardlr;
    volatile uint32_t idr;
    volatile uint32_t ssi_comp_version;
    volatile uint32_t dr; // Data register at 0x60
};

static struct dw_spi_regs *get_spi_base(uint8_t spi_id) {
    switch (spi_id) {
        case 0: return (struct dw_spi_regs *)SPI0_BASE;
        case 1: return (struct dw_spi_regs *)SPI1_BASE;
        case 2: return (struct dw_spi_regs *)SPI2_BASE;
        case 3: return (struct dw_spi_regs *)SPI3_BASE;
        default: return 0;
    }
}

void hal_spi_init(uint8_t spi_id, uint32_t max_speed_hz, uint8_t mode) {
    struct dw_spi_regs *spi = get_spi_base(spi_id);
    if (!spi) return;

    hal_clock_enable_spi(spi_id);
    hal_reset_deassert_spi(spi_id);

    // Disable SPI
    spi->ssienr = 0;

    // DFS=8 (data frame size), CPOL/CPHA from mode
    uint32_t scpol = (mode & 2) ? (1 << 7) : 0;
    uint32_t scph  = (mode & 1) ? (1 << 6) : 0;
    spi->ctrlr0 = 0x0007 | scpol | scph | (0 << 8) /* transmit/receive */;

    uint32_t div = SPI_INPUT_CLK / max_speed_hz;
    if (div < 2) div = 2;
    if (div > 65534) div = 65534;
    div &= 0xFFFE; // Must be even
    spi->baudr = div;

    // Enable Slave 0
    spi->ser = 1;
    
    // Enable SPI
    spi->ssienr = 1;
}

void hal_spi_transfer(uint8_t spi_id, const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len) {
    struct dw_spi_regs *spi = get_spi_base(spi_id);
    if (!spi) return;

    uint32_t tx_idx = 0;
    uint32_t rx_idx = 0;
    int timeout = 1000000;

    while (tx_idx < len || rx_idx < len) {
        if (--timeout == 0) {
            printf("[HAL] SPI%u Transfer Timeout! tx=%lu rx=%lu len=%lu\n", spi_id, tx_idx, rx_idx, len);
            return;
        }

        // Transmit if not full
        if (tx_idx < len && (spi->sr & (1 << 1))) { // TFNF
            spi->dr = tx_buf ? tx_buf[tx_idx] : 0xFF;
            tx_idx++;
            timeout = 1000000; // reset timeout on progress
        }
        
        // Receive if not empty
        if (rx_idx < tx_idx && (spi->sr & (1 << 3))) { // RFNE
            uint8_t data = spi->dr & 0xFF;
            if (rx_buf) rx_buf[rx_idx] = data;
            rx_idx++;
            timeout = 1000000; // reset timeout on progress
        }
    }

    // Wait until idle
    timeout = 100000;
    while ((spi->sr & (1 << 0))) { // BUSY
        for (int k=0; k<10; k++) asm volatile("nop");
        if (--timeout == 0) {
            printf("[HAL] SPI%u Busy Timeout!\n", spi_id);
            return;
        }
    }
}

