/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <delay.h>
#include <error.h>
#include <i2c.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <regulator/sy8106a.h>

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

static struct regulator_info *
sy8106a_get_info(struct device *dev __unused, uint8_t id __unused)
{
	assert(id < SY8106A_REGL_COUNT);

	return &sy8106a_regulator_info;
}

static int
sy8106a_get_state(struct device *dev, uint8_t id __unused)
{
	int     err;
	uint8_t reg;

	if ((err = i2c_read_reg(dev->bus, dev->addr, VOUT_COM_REG, &reg)))
		return err;

	return (reg & BIT(0)) == 0;
}

static int
sy8106a_read_raw(struct device *dev, uint8_t id __unused, uint32_t *raw)
{
	int     err;
	uint8_t reg;

	if ((err = i2c_read_reg(dev->bus, dev->addr, VOUT_SEL_REG, &reg)))
		return err;
	*raw = reg & ~BIT(7);

	return SUCCESS;
}

static int
sy8106a_set_state(struct device *dev, uint8_t id __unused, bool enabled)
{
	int     err;
	uint8_t reg;

	if ((err = i2c_read_reg(dev->bus, dev->addr, VOUT_COM_REG, &reg)))
		return err;
	reg = enabled ? reg & ~BIT(0) : reg | BIT(0);
	if ((err = i2c_write_reg(dev->bus, dev->addr, VOUT_COM_REG, reg)))
		return err;
	/* Wait for the regulator to start up (5 ms). */
	if (enabled)
		udelay(5000);

	return SUCCESS;
}

static int
sy8106a_write_raw(struct device *dev, uint8_t id __unused, uint32_t raw)
{
	assert(raw <= UINT8_MAX);

	return i2c_write_reg(dev->bus, dev->addr, VOUT_SEL_REG, raw | BIT(7));
}

static int
sy8106a_probe(struct device *dev)
{
	int err;
	uint16_t default_value = dev->drvdata;

	if ((err = i2c_probe(dev->bus, dev->addr)))
		return err;
	if ((err = regulator_set_defaults(dev, &default_value)))
		return err;

	dev->subdev_count = SY8106A_REGL_COUNT;

	return SUCCESS;
}

const struct regulator_driver sy8106a_driver = {
	.drv = {
		.class = DM_CLASS_REGULATOR,
		.probe = sy8106a_probe,
	},
	.ops = {
		.get_info  = sy8106a_get_info,
		.get_state = sy8106a_get_state,
		.read_raw  = sy8106a_read_raw,
		.set_state = sy8106a_set_state,
		.write_raw = sy8106a_write_raw,
	},
};
