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
			.id    = SUNXI_GPIO_PIN(1, 9), /* PB9 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(1, 10), /* PB10 */
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
			.id    = SUNXI_GPIO_PIN(1, 0), /* PB0 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(1, 1), /* PB1 */
			.drive = DRIVE_10mA,
			.mode  = 2,
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
			.id    = SUNXI_GPIO_PIN(6, 10), /* PG10 */
			.drive = DRIVE_10mA,
			.mode  = 3,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(6, 11), /* PG11 */
			.drive = DRIVE_10mA,
			.mode  = 3,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART3,
};

#elif CONFIG(SERIAL_DEV_UART4)

const struct simple_device uart = {
	.dev = {
		.name  = "uart4",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_UART4 },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(4, 10), /* PE10 */
			.drive = DRIVE_10mA,
			.mode  = 3,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(4, 11), /* PE11 */
			.drive = DRIVE_10mA,
			.mode  = 3,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART4,
};

#endif
