/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
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
			.id    = SUNXI_GPIO_PIN(7, 0), /* PH0 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(7, 1), /* PH1 */
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
			.id    = SUNXI_GPIO_PIN(3, 19), /* PD19 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(3, 20), /* PD20 */
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
			.id    = SUNXI_GPIO_PIN(3, 23), /* PD23 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &pio.dev,
			.id    = SUNXI_GPIO_PIN(3, 24), /* PD24 */
			.drive = DRIVE_10mA,
			.mode  = 4,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_UART3,
};

#elif CONFIG(SERIAL_DEV_R_UART)

const struct simple_device uart = {
	.dev = {
		.name  = "r_uart",
		.drv   = &uart_driver,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_UART },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 2), /* PL2 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_NONE,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 3), /* PL3 */
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_R_UART,
};

#endif
