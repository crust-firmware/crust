/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regmap.h>

#include "axp20x.h"

#define GPIO_LDO_MASK 0x7
#define GPIO_LDO_ON   0x3
#define GPIO_LDO_OFF  0x4

static int
axp20x_regulator_get_state(const struct device *dev, uint8_t id)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(dev);
	uint8_t reg  = self->info[id].enable_register;
	uint8_t mask = self->info[id].enable_mask;
	uint8_t val;
	int err;

	if ((err = regmap_read(self->map, reg, &val)))
		return err;

	/*
	 * GPIO LDOs have a pin function, not an enable bit. Their
	 * distinguishing feature is a mask containing more than one bit.
	 */
	if (mask == GPIO_LDO_MASK)
		return (val & mask) == GPIO_LDO_ON;

	return !!(val & mask);
}

static int
axp20x_regulator_set_state(const struct device *dev UNUSED, uint8_t id,
                           bool enabled)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(dev);
	uint8_t reg  = self->info[id].enable_register;
	uint8_t mask = self->info[id].enable_mask;
	uint8_t val;

	/*
	 * GPIO LDOs have a pin function, not an enable bit. Their
	 * distinguishing feature is a mask containing more than one bit.
	 */
	if (mask == GPIO_LDO_MASK)
		val = enabled ? GPIO_LDO_ON : GPIO_LDO_OFF;
	else
		val = enabled ? mask : 0;

	return regmap_update_bits(self->map, reg, mask, val);
}

static int
axp20x_regulator_probe(const struct device *dev)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(dev);

	return regmap_user_probe(self->map);
}

static void
axp20x_regulator_release(const struct device *dev)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(dev);

	regmap_user_release(self->map);
}

const struct regulator_driver axp20x_regulator_driver = {
	.drv = {
		.probe   = axp20x_regulator_probe,
		.release = axp20x_regulator_release,
	},
	.ops = {
		.get_state = axp20x_regulator_get_state,
		.set_state = axp20x_regulator_set_state,
	},
};
