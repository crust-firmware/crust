/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <division.h>
#include <error.h>
#include <mmio.h>

#include "uart.h"

static int
uart_probe(const struct device *dev)
{
	const struct simple_device *self = to_simple_device(dev);
	int err;

	if ((err = simple_device_probe(dev)))
		return err;

	if (CONFIG_SERIAL_BAUD) {
		uint32_t  rate    = clock_get_rate(&self->clock);
		uint32_t  divisor = udiv_round(rate, 16 * CONFIG_SERIAL_BAUD);
		uintptr_t regs    = self->regs;

		/* Set the clock divisor. */
		mmio_write_32(regs + UART_LCR, UART_LCR_DLAB);
		mmio_write_32(regs + UART_DLH, divisor >> 8);
		mmio_write_32(regs + UART_DLL, divisor);

		/* Set the UART to 8 data bits, no parity, 1 stop bit. */
		mmio_write_32(regs + UART_LCR, UART_LCR_DLS8);

		/* Enable the FIFOs. */
		mmio_write_32(regs + UART_FCR, UART_FCR_FIFOE);
	}

	return SUCCESS;
}

const struct driver uart_driver = {
	.probe   = uart_probe,
	.release = simple_device_release,
};
