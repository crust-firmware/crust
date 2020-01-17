/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_SUNXI_GPIO_H
#define DRIVERS_GPIO_SUNXI_GPIO_H

#include <clock.h>
#include <gpio.h>

#define SUNXI_GPIO_PIN(port, pin) (32 * (port) + (pin))

enum {
	DRIVE_10mA = 0,
	DRIVE_20mA = 1,
	DRIVE_30mA = 2,
	DRIVE_40mA = 3,
};

enum {
	MODE_INPUT   = 0,
	MODE_OUTPUT  = 1,
	MODE_DISABLE = 7,
};

enum {
	PULL_NONE = 0,
	PULL_UP   = 1,
	PULL_DOWN = 2,
};

struct sunxi_gpio {
	struct device       dev;
	struct clock_handle clock;
	uintptr_t           regs;
};

extern const struct sunxi_gpio r_pio;

#endif /* DRIVERS_GPIO_SUNXI_GPIO_H */
