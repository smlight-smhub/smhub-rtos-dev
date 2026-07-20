/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     trap_c.c
 * @brief    source file for the trap process
 * @version  V1.0
 * @date     12. December 2017
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//#include <csi_config.h>
#include "csi_core.h"
extern void uart_puts(char *str);

void (*trap_c_callback)(void);

void uart_put_hex_inline(uint64_t val) {
    uart_puts("0x");
    for (int i = 15; i >= 0; i--) {
        int nibble = (val >> (i * 4)) & 0xF;
        if (nibble < 10) {
            char c = '0' + nibble;
            extern uint8_t uart_putc(uint8_t ch);
            uart_putc(c);
        } else {
            char c = 'A' + (nibble - 10);
            extern uint8_t uart_putc(uint8_t ch);
            uart_putc(c);
        }
    }
}

void uart_put_hex(uint64_t val) {
    uart_put_hex_inline(val);
    uart_puts("\r\n");
}

void trap_c(uint64_t *regs)
{
    uint32_t vec = __get_MCAUSE() & 0x3FF;
    
    // Explicitly use uart_puts instead of printf to guarantee output without relying on standard library
    uart_puts("\r\n================================================\r\n");
    uart_puts("   FATAL HARDWARE EXCEPTION TRIGGERED!   \r\n");
    uart_puts("================================================\r\n");
    
    if (vec == 0) uart_puts("Cause: Instruction address misaligned\r\n");
    else if (vec == 1) uart_puts("Cause: Instruction access fault\r\n");
    else if (vec == 2) uart_puts("Cause: Illegal instruction\r\n");
    else if (vec == 3) uart_puts("Cause: Breakpoint\r\n");
    else if (vec == 4) uart_puts("Cause: Load address misaligned\r\n");
    else if (vec == 5) uart_puts("Cause: Load access fault\r\n");
    else if (vec == 6) uart_puts("Cause: Store/AMO address misaligned\r\n");
    else if (vec == 7) uart_puts("Cause: Store/AMO access fault\r\n");
    else uart_puts("Cause: Unknown (check mcause)\r\n");

    uint64_t mepc = 0;
    uint64_t mtval = 0;
    uint64_t mcause = 0;
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile ("csrr %0, mtval" : "=r"(mtval));
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause));

    uart_puts("MEPC:   ");
    uart_put_hex(mepc);
    uart_puts("MTVAL:  ");
    uart_put_hex(mtval);
    uart_puts("MCAUSE: ");
    uart_put_hex(mcause);

    // regs[0] = x1 (ra), regs[1] = x2 (sp), ..., regs[6] = x8 (s0/fp)
    uart_puts("\r\n--- Stack Trace (Run with addr2line) ---\r\n");
    uint64_t fp = regs[6];
    int depth = 0;
    
    while (fp != 0 && depth < 20) {
        // Ensure FP is 8-byte aligned and roughly within RTOS RAM bounds (e.g. 0x80000000+)
        if ((fp & 0x7) != 0 || fp < 0x80000000) {
            break;
        }
        
        // RISC-V GCC Frame Layout:
        // FP - 8  : Return Address (ra)
        // FP - 16 : Previous Frame Pointer (fp)
        uint64_t ra = *((uint64_t*)(fp - 8));
        uint64_t next_fp = *((uint64_t*)(fp - 16));
        
        uart_put_hex_inline(ra);
        uart_puts(" ");
        
        // Break if frame pointer isn't strictly increasing (prevents infinite loops)
        if (next_fp <= fp) break;
        
        fp = next_fp;
        depth++;
    }
    uart_puts("\r\n================================================\r\n");

    if (trap_c_callback) {
        trap_c_callback();
    }

    while (1);
}

