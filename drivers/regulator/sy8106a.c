/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <regmap.h>
#include <regulator.h>
#include <util.h>
#include <regmap/sun6i-i2c.h>
#include <regulator/sy8106a.h>

#include "regulator.h"

#define VOUT_SEL_REG   0x01
#define VOUT_COM_REG   0x02
#define SYS_STATUS_REG 0x06

static int
sy8106a_get_state(const struct device *dev, uint8_t id UNUSED)
{
	const struct regmap_device *self = to_regmap_device(dev);
	uint8_t val;
	int err;

	if ((err = regmap_read(&self->map, VOUT_COM_REG, &val)))
		return err;

	return !(val & BIT(0));
}

static int
sy8106a_set_state(const struct device *dev, uint8_t id UNUSED, bool enabled)
{
	const struct regmap_device *self = to_regmap_device(dev);
	int err;

	if ((err = regmap_update_bits(&self->map, VOUT_COM_REG,
	                              BIT(0), !enabled)))
		return err;

	return SUCCESS;
}

static const struct regulator_driver sy8106a_driver = {
	.drv = {
		.probe   = regmap_device_probe,
		.release = regmap_device_release,
	},
	.ops = {
		.get_state = sy8106a_get_state,
		.set_state = sy8106a_set_state,
	},
};

const struct regmap_device sy8106a = {
	.dev = {
		.name  = "sy8106a",
		.drv   = &sy8106a_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map = {
		.dev = &r_i2c.dev,
		.id  = SY8106A_I2C_ADDRESS,
	},
};
