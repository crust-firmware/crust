/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_AXP20X_H
#define DRIVERS_PMIC_AXP20X_H

#include <device.h>
#include <pmic.h>
#include <regmap.h>

struct axp20x_pmic {
	struct device        dev;
	const struct regmap *map;
};

#endif /* DRIVERS_PMIC_AXP20X_H */
