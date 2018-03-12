/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_REGULATOR_AXP803_H
#define DRIVERS_REGULATOR_AXP803_H

#include <regulator.h>

struct axp803_regulator_info {
	struct regulator_info info;
	uint8_t               value_register;
	uint8_t               enable_register;
	uint8_t               bit;
};

extern const struct regulator_driver axp803_driver;

#endif /* DRIVERS_REGULATOR_AXP803_H */
