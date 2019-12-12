/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_SUNXI_GPIO_H
#define DRIVERS_GPIO_SUNXI_GPIO_H

#include <clock.h>
#include <gpio.h>

#define SUNXI_GPIO_PIN(port, index) (32 * (port) + (index))

struct sunxi_gpio {
	struct device       dev;
	struct clock_handle clock;
	uintptr_t           regs;
};

extern struct sunxi_gpio r_pio;

#endif /* DRIVERS_GPIO_SUNXI_GPIO_H */
