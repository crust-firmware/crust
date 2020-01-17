/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <delay.h>
#include <error.h>
#include <i2c.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <i2c/sun6i-i2c.h>
#include <regulator/sy8106a.h>

#include "regulator.h"

#define VOUT_SEL_REG   0x01
#define VOUT_COM_REG   0x02
#define SYS_STATUS_REG 0x06

static struct regulator_info sy8106a_regulator_info = {
	.name      = "vout",
	.min_value = 680,
	.max_value = 1950,
	.ranges    = {
		{
			.start_raw   = 0x00,
			.start_value = 680,
			.step        = 10,
		},
	},
	.flags = REGL_READABLE,
};

static inline const struct sy8106a *
to_sy8106a(const struct device *dev)
{
	return container_of(dev, const struct sy8106a, dev);
}

static struct regulator_info *
sy8106a_get_info(const struct device *dev UNUSED, uint8_t id UNUSED)
{
	assert(id < SY8106A_REGL_COUNT);

	return &sy8106a_regulator_info;
}

static int
sy8106a_get_state(const struct device *dev, uint8_t id UNUSED)
{
	const struct sy8106a *self = to_sy8106a(dev);
	uint8_t reg;
	int err;

	if ((err = i2c_read_reg(&self->bus, VOUT_COM_REG, &reg)))
		return err;

	return (reg & BIT(0)) == 0;
}

static int
sy8106a_read_raw(const struct device *dev, uint8_t id UNUSED, uint32_t *raw)
{
	const struct sy8106a *self = to_sy8106a(dev);
	uint8_t reg;
	int err;

	if ((err = i2c_read_reg(&self->bus, VOUT_SEL_REG, &reg)))
		return err;
	*raw = reg & ~BIT(7);

	return SUCCESS;
}

static int
sy8106a_set_state(const struct device *dev, uint8_t id UNUSED, bool enabled)
{
	const struct sy8106a *self = to_sy8106a(dev);
	uint8_t reg;
	int err;

	if ((err = i2c_read_reg(&self->bus, VOUT_COM_REG, &reg)))
		return err;
	reg = enabled ? reg & ~BIT(0) : reg | BIT(0);
	if ((err = i2c_write_reg(&self->bus, VOUT_COM_REG, reg)))
		return err;
	/* Wait for the regulator to start up (5 ms). */
	if (enabled)
		udelay(5000);

	return SUCCESS;
}

static int
sy8106a_write_raw(const struct device *dev, uint8_t id UNUSED, uint32_t raw)
{
	const struct sy8106a *self = to_sy8106a(dev);

	assert(raw <= UINT8_MAX);

	return i2c_write_reg(&self->bus, VOUT_SEL_REG, raw | BIT(7));
}

static int
sy8106a_probe(const struct device *dev)
{
	const struct sy8106a *self = to_sy8106a(dev);
	int err;

	if ((err = i2c_get(&self->bus)))
		return err;

	return SUCCESS;
}

static void
sy8106a_release(const struct device *dev)
{
	const struct sy8106a *self = to_sy8106a(dev);

	i2c_put(&self->bus);
}

static const struct regulator_driver sy8106a_driver = {
	.drv = {
		.probe   = sy8106a_probe,
		.release = sy8106a_release,
	},
	.ops = {
		.get_info  = sy8106a_get_info,
		.get_state = sy8106a_get_state,
		.read_raw  = sy8106a_read_raw,
		.set_state = sy8106a_set_state,
		.write_raw = sy8106a_write_raw,
	},
};

const struct sy8106a sy8106a = {
	.dev = {
		.name  = "sy8106a",
		.drv   = &sy8106a_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.bus = {
		.dev = &r_i2c.dev,
		.id  = SY8106A_I2C_ADDRESS,
	},
};
