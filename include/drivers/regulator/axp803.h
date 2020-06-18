/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_AXP803_H
#define DRIVERS_REGULATOR_AXP803_H

#include <device.h>
#include <regmap.h>
#include <regulator.h>

enum {
	AXP803_REGL_DCDC1,
	AXP803_REGL_DCDC2,
	AXP803_REGL_DCDC3,
	AXP803_REGL_DCDC4,
	AXP803_REGL_DCDC5,
	AXP803_REGL_DCDC6,
	AXP803_REGL_DC1SW,
	AXP803_REGL_ALDO1,
	AXP803_REGL_ALDO2,
	AXP803_REGL_ALDO3,
	AXP803_REGL_DLDO1,
	AXP803_REGL_DLDO2,
	AXP803_REGL_DLDO3,
	AXP803_REGL_DLDO4,
	AXP803_REGL_ELDO1,
	AXP803_REGL_ELDO2,
	AXP803_REGL_ELDO3,
	AXP803_REGL_FLDO1,
	AXP803_REGL_FLDO2,
	AXP803_REGL_GPIO0,
	AXP803_REGL_GPIO1,
	AXP803_REGL_COUNT,
};

struct axp803_regulator {
	struct device                       dev;
	const struct regmap                *map;
	const struct axp803_regulator_info *info;
};

extern const struct axp803_regulator axp803_regulator;

#endif /* DRIVERS_REGULATOR_AXP803_H */
