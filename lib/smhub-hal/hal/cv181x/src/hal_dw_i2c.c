/*
 * Copyright (C) 2026 SMLIGHT
 * All rights reserved.
 *
 * This file is part of the SMHUB RTOS project.
 */
#include "hal_dw_i2c.h"
#include "mmio.h"
#include "irq.h"
#include "cv181x_interrupts.h"
#include "cv181x_top_reg.h"
#include "hal_clock.h"
#include "smhub_arbitration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

#define DW_I2C_CON          0x00
#define DW_I2C_TAR          0x04
#define DW_I2C_DATA_CMD     0x10
#define DW_I2C_SS_SCL_HCNT  0x14
#define DW_I2C_SS_SCL_LCNT  0x18
#define DW_I2C_FS_SCL_HCNT  0x1C
#define DW_I2C_FS_SCL_LCNT  0x20
#define DW_I2C_INTR_MASK    0x30
#define DW_I2C_RAW_INTR_STAT 0x34
#define DW_I2C_CLR_INTR     0x40
#define DW_I2C_ENABLE       0x6C
#define DW_I2C_STATUS       0x70

#define IC_CON_MASTER_MODE      (1 << 0)
#define IC_CON_SPEED_STD        (1 << 1)
#define IC_CON_SPEED_FAST       (2 << 1)
#define IC_CON_RESTART_EN       (1 << 5)
#define IC_CON_SLAVE_DISABLE    (1 << 6)

#define IC_STATUS_ACTIVITY      (1 << 0)
#define IC_STATUS_TFNF          (1 << 1)
#define IC_STATUS_TFE           (1 << 2)
#define IC_STATUS_RFNE          (1 << 3)

#define IC_CMD_READ             (1 << 8)
#define IC_CMD_STOP             (1 << 9)

#ifndef I2C_USE_INTERRUPTS
#define I2C_USE_INTERRUPTS 0
#endif

static uintptr_t get_i2c_base(uint8_t i2c_id) {
    switch (i2c_id) {
        case 0: return I2C0_BASE;
        case 1: return I2C1_BASE;
        case 2: return I2C2_BASE;
        case 3: return I2C3_BASE;
        case 4: return I2C4_BASE;
        default: return 0;
    }
}

static void i2c_enable(uintptr_t base, bool en) {
    uint32_t ena = en ? 1 : 0;
    int timeout = 1000;
    mmio_write_32(base + DW_I2C_ENABLE, ena);
    while (--timeout > 0) {
        if ((mmio_read_32(base + 0x9C) & 1) == ena) break;
        for (int i = 0; i < 100; i++) asm volatile("nop");
    }
}

static bool wait_tx_not_full(uintptr_t base) {
    int timeout = 100000;
    while (!(mmio_read_32(base + DW_I2C_STATUS) & IC_STATUS_TFNF)) {
        for (int k=0; k<10; k++) asm volatile("nop");
        if (--timeout == 0) return false;
    }
    return true;
}

static bool wait_rx_not_empty(uintptr_t base) {
    int timeout = 100000;
    while (!(mmio_read_32(base + DW_I2C_STATUS) & IC_STATUS_RFNE)) {
        for (int k=0; k<10; k++) asm volatile("nop");
        if (mmio_read_32(base + DW_I2C_RAW_INTR_STAT) & (1 << 6)) return false; // Abort
        if (--timeout == 0) return false;
    }
    return true;
}

static bool wait_idle(uintptr_t base) {
    int timeout = 100000;
    // Vendor Quirk: Check for Master Activity and TX FIFO Empty
    while ((mmio_read_32(base + DW_I2C_STATUS) & (1 << 5)) ||
           !(mmio_read_32(base + DW_I2C_STATUS) & IC_STATUS_TFE)) {
        for (int k=0; k<10; k++) asm volatile("nop");
        if (--timeout == 0) return false;
    }
    return true;
}

static bool i2c_xfer_finish(uintptr_t base) {
    int timeout = 100000;
    // Wait for IC_STOP_DET (bit 9)
    while (!(mmio_read_32(base + DW_I2C_RAW_INTR_STAT) & (1 << 9))) {
        for (int k=0; k<10; k++) asm volatile("nop");
        if (--timeout == 0) return false;
    }
    // Clear IC_STOP_DET
    mmio_read_32(base + 0x60); // DW_I2C_CLR_STOP_DET
    return wait_idle(base);
}

void hal_i2c_set_frequency(uint8_t i2c_id, uint32_t frequency) {
    uintptr_t base = get_i2c_base(i2c_id);
    if (!base) return;
    wait_idle(base);
    i2c_enable(base, false);
    uint32_t cntl = mmio_read_32(base + DW_I2C_CON) & ~(3 << 1);

    if (frequency <= 100000) {
        // Standard Mode (100kHz)
        cntl |= IC_CON_SPEED_STD;
        mmio_write_32(base + DW_I2C_SS_SCL_HCNT, 393);
        mmio_write_32(base + DW_I2C_SS_SCL_LCNT, 469);
    } else {
        cntl |= IC_CON_SPEED_FAST;
        mmio_write_32(base + DW_I2C_FS_SCL_HCNT, 60);
        mmio_write_32(base + DW_I2C_FS_SCL_LCNT, 130);
    }

    mmio_write_32(base + DW_I2C_CON, cntl);
    i2c_enable(base, true);
}

void hal_i2c_init(uint8_t i2c_id) {
    uintptr_t base = get_i2c_base(i2c_id);
    if (!base) return;

    uint32_t hw_bit = 0;
    if (i2c_id == 2) hw_bit = SMHUB_HW_I2C2;
    else if (i2c_id == 4) hw_bit = SMHUB_HW_I2C4;
    if (hw_bit && !smhub_hardware_is_released(hw_bit)) {
        printf("[HAL] I2C%u Hardware Arbitration FAILED!\n", i2c_id);
        return;
    }

    hal_clock_enable_i2c(i2c_id);
    hal_reset_i2c(i2c_id);

    i2c_enable(base, false);
    mmio_write_32(base + DW_I2C_CON, IC_CON_MASTER_MODE | IC_CON_SPEED_FAST | IC_CON_RESTART_EN | IC_CON_SLAVE_DISABLE);
    mmio_write_32(base + DW_I2C_INTR_MASK, 0x00);
    mmio_write_32(base + DW_I2C_FS_SCL_HCNT, 60);
    mmio_write_32(base + DW_I2C_FS_SCL_LCNT, 130);
    mmio_write_32(base + 0x38, 0); // DW_I2C_RX_TL
}

int hal_i2c_xfer(uint8_t i2c_id, struct i2c_msg msgs[], int num) {
    static int timeout_count[5] = {0};
    uintptr_t base = get_i2c_base(i2c_id);
    if (!base) return -1;
    if (num == 0) return 0;

    i2c_enable(base, false);
    mmio_write_32(base + DW_I2C_TAR, msgs[0].addr);
    i2c_enable(base, true);
    mmio_read_32(base + 0x54); // CLR_TX_ABRT

    for (int i = 0; i < num; i++) {
        struct i2c_msg *msg = &msgs[i];
        bool is_last_msg = (i == num - 1);

        if (msg->flags & I2C_M_RD) {
            int rx_len = msg->len;
            int tx_len = msg->len;
            uint8_t *buf = msg->buf;
            int total_timeout = 1000000;

            while (rx_len > 0) {
                if (--total_timeout == 0) {
                    printf("[HAL] I2C%u Read Deadlock Timeout! rx_len=%d tx_len=%d\n", i2c_id, rx_len, tx_len);
                    goto err_out;
                }

                // Push read commands if TX FIFO is not full
                while (tx_len > 0 && (mmio_read_32(base + DW_I2C_STATUS) & (1 << 1))) {
                    uint32_t cmd = IC_CMD_READ;
                    if (tx_len == 1 && is_last_msg) cmd |= IC_CMD_STOP;
                    mmio_write_32(base + DW_I2C_DATA_CMD, cmd);
                    tx_len--;
                }

                // Pop received bytes if RX FIFO is not empty
                while (rx_len > 0 && (mmio_read_32(base + DW_I2C_STATUS) & IC_STATUS_RFNE)) {
                    *buf++ = mmio_read_32(base + DW_I2C_DATA_CMD) & 0xFF;
                    rx_len--;
                }

                if (mmio_read_32(base + DW_I2C_RAW_INTR_STAT) & (1 << 6)) goto err_out; // TX_ABRT
            }
        } else {
            for (int j = 0; j < msg->len; j++) {
                int tfnf_timeout = 100000;
                while (!(mmio_read_32(base + DW_I2C_STATUS) & (1 << 1))) { // TFNF
                    if (--tfnf_timeout == 0) {
                        printf("[HAL] I2C%u TFNF Timeout Write!\n", i2c_id);
                        goto err_out;
                    }
                    if (mmio_read_32(base + DW_I2C_RAW_INTR_STAT) & (1 << 6)) goto err_out;
                }
                uint32_t cmd = msg->buf[j];
                if (j == msg->len - 1 && is_last_msg) cmd |= IC_CMD_STOP;
                mmio_write_32(base + DW_I2C_DATA_CMD, cmd);
            }
        }
    }

    if (!i2c_xfer_finish(base)) {
        printf("[HAL] I2C%u Finish Timeout!\n", i2c_id);
        goto err_out;
    }

    if (i2c_id < 5) {
        timeout_count[i2c_id] = 0;
    }
    return 0;

err_out:
    {
        uint32_t abrt_src = mmio_read_32(base + 0x80);
        if (abrt_src != 1) { // 1 = ABRT_7B_ADDR_NOACK
            printf("[HAL] I2C%u Hardware Error! TX_ABRT_SOURCE: 0x%08X\n", i2c_id, abrt_src);
        }
        mmio_read_32(base + 0x54); // CLR_TX_ABRT

        if (i2c_id < 5) {
            if (abrt_src == 0) {
                printf("[HAL] I2C%u: Real hardware timeout, resetting hardware immediately...\n", i2c_id);
                hal_i2c_init(i2c_id);
                timeout_count[i2c_id] = 0;
            } else if (abrt_src != 1) {
                timeout_count[i2c_id]++;
                if (timeout_count[i2c_id] >= 3) {
                    printf("[HAL] I2C%u: %d consecutive hardware errors, resetting hardware...\n", i2c_id, timeout_count[i2c_id]);
                    hal_i2c_init(i2c_id);
                    timeout_count[i2c_id] = 0;
                }
            }
        }

        return (abrt_src != 0) ? (int)abrt_src : -1;
    }
}
