/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef UART_PRIVATE_H
#define UART_PRIVATE_H

#include <simple_device.h>
#include <util.h>

enum {
	UART_RBR = 0x0000,
	UART_THR = 0x0000,
	UART_DLL = 0x0000,
	UART_DLH = 0x0004,
	UART_FCR = 0x0008,
	UART_LCR = 0x000c,
	UART_LSR = 0x0014,
};

enum {
	UART_FCR_FIFOE = BIT(0),
};

enum {
	UART_LCR_DLAB = BIT(7),
	UART_LCR_DLS8 = GENMASK(1, 0),
};

enum {
	UART_LSR_DR   = BIT(0),
	UART_LSR_THRE = BIT(5),
};

extern const struct driver uart_driver;
extern const struct simple_device uart;

#endif /* UART_PRIVATE_H */
