/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <regmap.h>
#include <stdint.h>

#include "regmap.h"

/**
 * Get the ops for the regmap device.
 */
static inline const struct regmap_driver_ops *
regmap_ops_for(const struct regmap *map)
{
	const struct regmap_driver *drv =
		container_of(map->dev->drv, const struct regmap_driver, drv);

	return &drv->ops;
}

int
regmap_get(const struct regmap *map)
{
	int err;

	/* Ensure the controller's driver is loaded. */
	if (!device_get(map->dev))
		return ENODEV;

	if ((err = regmap_ops_for(map)->prepare(map)))
		goto err_put_device;

	return SUCCESS;

err_put_device:
	device_put(map->dev);

	return err;
}

void
regmap_put(const struct regmap *map)
{
	device_put(map->dev);
}

int
regmap_read(const struct regmap *map, uint8_t reg, uint8_t *val)
{
	return regmap_ops_for(map)->read(map, reg, val);
}

int
regmap_set_bits(const struct regmap *map, uint8_t reg, uint8_t set)
{
	const struct regmap_driver_ops *ops = regmap_ops_for(map);
	uint8_t val;
	int err;

	if ((err = ops->read(map, reg, &val)))
		return err;

	return ops->write(map, reg, val | set);
}

int
regmap_write(const struct regmap *map, uint8_t reg, uint8_t val)
{
	return regmap_ops_for(map)->write(map, reg, val);
}
