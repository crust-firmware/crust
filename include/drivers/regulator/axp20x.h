/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_AXP20X_H
#define DRIVERS_REGULATOR_AXP20X_H

#include <device.h>
#include <regmap.h>
#include <regulator.h>

struct axp20x_regulator {
	struct device                       dev;
	const struct regmap                *map;
	const struct axp20x_regulator_info *info;
};

#endif /* DRIVERS_REGULATOR_AXP20X_H */
