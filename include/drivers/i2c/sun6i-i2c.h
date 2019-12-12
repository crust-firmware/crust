/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_I2C_SUN6I_I2C_H
#define DRIVERS_I2C_SUN6I_I2C_H

#include <clock.h>
#include <gpio.h>
#include <i2c.h>

struct sun6i_i2c {
	struct device       dev;
	struct clock_handle clock;
	struct gpio_handle  pins[I2C_NUM_PINS];
	uintptr_t           regs;
};

extern const struct sun6i_i2c r_i2c;

#endif /* DRIVERS_I2C_SUN6I_I2C_H */
