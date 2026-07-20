#include <stdint.h>
#include "hal_uart_dw.h"
#include "cv181x_top_reg.h"
#include "smhub_arbitration.h"

static struct dw_regs *get_uart_base(uint8_t uart_id) {
	switch (uart_id) {
		case 0: return (struct dw_regs *)UART0_BASE;
		case 1: return (struct dw_regs *)UART1_BASE;
		case 2: return (struct dw_regs *)UART2_BASE;
		case 3: return (struct dw_regs *)UART3_BASE;
		default: return 0;
	}
}

void hal_uart_init(uint8_t uart_id, uint32_t baudrate, uint32_t uart_clock)
{
	if (uart_id == 2 && !smhub_hardware_is_released(SMHUB_HW_UART2)) return;
	if (uart_id == 3 && !smhub_hardware_is_released(SMHUB_HW_UART3)) return;

	struct dw_regs *uart = get_uart_base(uart_id);
	if (!uart) return;

	int divisor = (uart_clock + 8 * baudrate) / (16 * baudrate);

	uart->lcr = uart->lcr | UART_LCR_DLAB | UART_LCR_8N1;
	uart->dll = divisor & 0xff;
	uart->dlm = (divisor >> 8) & 0xff;
	uart->lcr = uart->lcr & (~UART_LCR_DLAB);

	uart->ier = 0;
	uart->mcr = UART_MCRVAL;
	uart->fcr = UART_FCR_DEFVAL;

	uart->lcr = 3;
}

void hal_uart_putc(uint8_t uart_id, uint8_t ch)
{
	struct dw_regs *uart = get_uart_base(uart_id);
	if (!uart) return;
	while (!(uart->lsr & UART_LSR_THRE))
		;
	uart->rbr = ch;
}

int hal_uart_getc(uint8_t uart_id)
{
	struct dw_regs *uart = get_uart_base(uart_id);
	if (!uart) return -1;
	while (!(uart->lsr & UART_LSR_DR))
		;
	return (int)uart->rbr;
}

int hal_uart_tstc(uint8_t uart_id)
{
	struct dw_regs *uart = get_uart_base(uart_id);
	if (!uart) return 0;
	return (!!(uart->lsr & UART_LSR_DR));
}
