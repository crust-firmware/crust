/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef UART_PRIVATE_H
#define UART_PRIVATE_H

#include <simple_device.h>
#include <util.h>

enum {
	UART_RBR = 0x0000,
	UART_THR = 0x0000,
	UART_LSR = 0x0014,
};

enum {
	UART_LSR_DR   = BIT(0),
	UART_LSR_THRE = BIT(5),
};

extern const struct driver uart_driver;
extern const struct simple_device uart;

#endif /* UART_PRIVATE_H */
