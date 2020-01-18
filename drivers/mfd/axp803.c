/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <regmap.h>
#include <stdbool.h>
#include <stdint.h>
#include <mfd/axp803.h>
#include <regmap/sunxi-rsb.h>

#define IC_TYPE_REG       0x03
#define IC_TYPE_MASK      0xcf
#define IC_TYPE_VALUE     0x41

#define AXP803_RSB_RTADDR 0x2d

static inline const struct axp803 *
to_axp803(const struct device *dev)
{
	return container_of(dev, const struct axp803, dev);
}

static int
axp803_probe(const struct device *dev)
{
	const struct axp803 *self = to_axp803(dev);
	uint8_t reg;
	int err;

	if ((err = regmap_get(&self->map)))
		return err;
	if ((err = regmap_read(&self->map, IC_TYPE_REG, &reg)))
		goto err_put_regmap;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE) {
		err = ENODEV;
		goto err_put_regmap;
	}

	return SUCCESS;

err_put_regmap:
	regmap_put(&self->map);

	return err;
}

static void
axp803_release(const struct device *dev)
{
	const struct axp803 *self = to_axp803(dev);

	regmap_put(&self->map);
}

static const struct driver axp803_driver = {
	.probe   = axp803_probe,
	.release = axp803_release,
};

const struct axp803 axp803 = {
	.dev = {
		.name  = "axp803",
		.drv   = &axp803_driver,
		.state = DEVICE_STATE_INIT,
	},
	.map = {
		.dev = &r_rsb.dev,
		.id  = AXP803_RSB_RTADDR,
	},
};

int
axp803_subdevice_probe(const struct device *dev UNUSED)
{
	return device_get(&axp803.dev) ? SUCCESS : ENODEV;
}

void
axp803_subdevice_release(const struct device *dev UNUSED)
{
	device_put(&axp803.dev);
}
