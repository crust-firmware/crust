/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef AXP20X_PRIVATE_H
#define AXP20X_PRIVATE_H

#include <intrusive.h>
#include <stdint.h>
#include <regulator/axp20x.h>

#include "regulator.h"

struct axp20x_regulator_info {
	uint8_t enable_register;
	uint8_t enable_mask;
	uint8_t value_register;
	uint8_t value_mask;
};

extern const struct regulator_driver axp20x_regulator_driver;

static inline const struct axp20x_regulator *
to_axp20x_regulator(const struct device *dev)
{
	return container_of(dev, const struct axp20x_regulator, dev);
}

#endif /* AXP20X_PRIVATE_H */
