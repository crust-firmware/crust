/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <platform/devices.h>

#include "uart.h"

#if CONFIG(SERIAL_DEV_UART0)

const struct simple_device uart = {
	.dev = {
		.name  = "uart0",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_UART0 },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 4), /* PA4 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 5), /* PA5 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART0,
};

#elif CONFIG(SERIAL_DEV_UART1)

const struct simple_device uart = {
	.dev = {
		.name  = "uart1",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_UART1 },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(6, 6), /* PG6 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(6, 7), /* PG7 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART1,
};

#elif CONFIG(SERIAL_DEV_UART2)

const struct simple_device uart = {
	.dev = {
		.name  = "uart2",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_UART2 },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 0), /* PA0 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 1), /* PA1 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART2,
};

#elif CONFIG(SERIAL_DEV_UART3)

const struct simple_device uart = {
	.dev = {
		.name  = "uart3",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_UART3 },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 13), /* PA13 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 14), /* PA14 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART3,
};

#endif
