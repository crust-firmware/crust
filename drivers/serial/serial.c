/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <mmio.h>
#include <serial.h>

#include "uart.h"

char
serial_getc(void)
{
	if (!mmio_get_32(uart.regs + UART_LSR, UART_LSR_DR))
		return 0;

	return mmio_read_32(uart.regs + UART_RBR);
}

void
serial_putc(char c)
{
	if (c == '\n')
		serial_putc('\r');
	mmio_poll_32(uart.regs + UART_LSR, UART_LSR_THRE);
	mmio_write_32(uart.regs + UART_THR, c);
}

void
serial_puts(const char *s)
{
	char c;

	while ((c = *s++))
		serial_putc(c);
}

void
serial_init(void)
{
	device_get(&uart.dev);
}

bool
serial_ready(void)
{
	bool ready = device_active(&uart.dev);

	/*
	 * If the UART is shared with other users, its clock may have been
	 * gated. Ensure the clock is running before accessing the device.
	 */
	if (ready)
		clock_enable(&uart.clock);

	return ready;
}
