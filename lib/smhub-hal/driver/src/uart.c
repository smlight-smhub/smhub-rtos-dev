/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * UART console interface and Linux diagnostic buffer implementation.
 */
#include "hal_uart_dw.h"
#include "hal_pinmux.h"
#include <stdint.h>
#include <stdbool.h>
#include <types.h>
#include <stdio.h>
#include <string.h>

static uint32_t ram_log_offset = 0; // Starts at 0x8ffe0000

void uart_init(void)
{
	// Clear the 4KB dummy RAM UART buffer to prevent garbage characters from previous boots
	ram_log_offset = 0;
	memset((void *)0x8ffe0000, 0, 4096);
	
	// Intentionally empty to prevent scrambling Linux UART0
}

uint8_t uart_putc(uint8_t ch)
{
	// Write to dedicated RTOS RAM instead of AXI Mailbox!
	volatile uint8_t *log_ram = (volatile uint8_t *)0x8ffe0000;

	if (ch == '\n') {
		log_ram[ram_log_offset++] = '\r';
		if (ram_log_offset >= 4096) ram_log_offset = 0;
	}
	
	log_ram[ram_log_offset++] = ch;
	if (ram_log_offset >= 4096) ram_log_offset = 0;

	// Null terminate
	log_ram[ram_log_offset] = '\0';

	return ch;
}

extern void flush_dcache_range(uintptr_t start, size_t size);

void uart_puts(char *str)
{
	if (!str)
		return;

	while (*str) {
		uart_putc(*str++);
	}
	
	// Flush the entire 4KB log buffer to physical RAM so the Linux host can read it via /dev/mem
	flush_dcache_range(0x8ffe0000, 4096);
}

int uart_getc(void)
{
	return (int)hal_uart_getc(0);
}

int uart_tstc(void)
{
	return hal_uart_tstc(0);
}

int uart_put_buff(char *buf)
{
	int count = 0;

	uart_puts("RT: ");

	while (buf[count]) {
		if (uart_putc(buf[count]) != '\n') {
			count++;
		} else {
			break;
		}
	}

	return count;
}

// Wire up the newlib printf syscall to our UART hardware
#include <reent.h>
_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len)
{
	const char *cptr = (const char *)ptr;
	size_t i;
	for (i = 0; i < len; i++) {
		uart_putc(cptr[i]);
	}
	return len;
}
