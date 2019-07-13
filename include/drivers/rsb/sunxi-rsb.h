/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_RSB_SUNXI_RSB_H
#define DRIVERS_RSB_SUNXI_RSB_H

#include <clock.h>
#include <gpio.h>
#include <rsb.h>

struct sunxi_rsb {
	struct device       dev;
	struct clock_handle clock;
	struct gpio_handle  pins[RSB_NUM_PINS];
};

extern struct sunxi_rsb r_rsb;

#endif /* DRIVERS_RSB_SUNXI_RSB_H */
