/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGMAP_SUN6I_I2C_H
#define DRIVERS_REGMAP_SUN6I_I2C_H

#include <clock.h>
#include <gpio.h>
#include <regmap.h>

struct sun6i_i2c {
	struct device       dev;
	struct clock_handle clock;
	struct gpio_handle  pins[2];
	uintptr_t           regs;
};

extern const struct sun6i_i2c r_i2c;

#endif /* DRIVERS_REGMAP_SUN6I_I2C_H */
