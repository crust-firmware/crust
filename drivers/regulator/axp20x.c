/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <regmap.h>

#include "axp20x.h"

static int
axp20x_regulator_get_state(const struct regulator_handle *handle,
                           bool *enabled)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(handle->dev);
	uint8_t addr = self->info[handle->id].enable_register;
	uint8_t mask = self->info[handle->id].enable_mask;
	uint8_t val;
	int err;

	if ((err = regmap_read(self->map, addr, &val)))
		return err;

	*enabled = (val & mask);

	return SUCCESS;
}

static int
axp20x_regulator_set_state(const struct regulator_handle *handle, bool enabled)
{
	const struct axp20x_regulator *self = to_axp20x_regulator(handle->dev);
	uint8_t addr = self->info[handle->id].enable_register;
	uint8_t mask = self->info[handle->id].enable_mask;

	return regmap_update_bits(self->map, addr, mask, enabled ? mask : 0);
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
