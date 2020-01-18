/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <error.h>
#include <intrusive.h>
#include <regmap.h>
#include <regulator.h>
#include <util.h>
#include <regmap/sun6i-i2c.h>
#include <regulator/sy8106a.h>

#include "regulator.h"

#define VOUT_SEL_REG   0x01
#define VOUT_COM_REG   0x02
#define SYS_STATUS_REG 0x06

static inline const struct sy8106a *
to_sy8106a(const struct device *dev)
{
	return container_of(dev, const struct sy8106a, dev);
}

static int
sy8106a_get_state(const struct device *dev, uint8_t id UNUSED)
{
	const struct sy8106a *self = to_sy8106a(dev);
	uint8_t val;
	int err;

	if ((err = regmap_read(&self->map, VOUT_COM_REG, &val)))
		return err;

	return !(val & BIT(0));
}

static int
sy8106a_set_state(const struct device *dev, uint8_t id UNUSED, bool enabled)
{
	const struct sy8106a *self = to_sy8106a(dev);
	int err;

	if ((err = regmap_update_bits(&self->map, VOUT_COM_REG,
	                              BIT(0), !enabled)))
		return err;

	/* Wait for the regulator to start up (5 ms). */
	if (enabled)
		udelay(5000);

	return SUCCESS;
}

static int
sy8106a_probe(const struct device *dev)
{
	const struct sy8106a *self = to_sy8106a(dev);
	int err;

	if ((err = regmap_get(&self->map)))
		return err;

	return SUCCESS;
}

static void
sy8106a_release(const struct device *dev)
{
	const struct sy8106a *self = to_sy8106a(dev);

	regmap_put(&self->map);
}

static const struct regulator_driver sy8106a_driver = {
	.drv = {
		.probe   = sy8106a_probe,
		.release = sy8106a_release,
	},
	.ops = {
		.get_state = sy8106a_get_state,
		.set_state = sy8106a_set_state,
	},
};

const struct sy8106a sy8106a = {
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
