/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <platform/devices.h>

#include "uart.h"

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
