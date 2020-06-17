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
	if ((err = device_get(map->dev)))
		return err;

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
regmap_write(const struct regmap *map, uint8_t reg, uint8_t val)
{
	return regmap_ops_for(map)->write(map, reg, val);
}

int
regmap_update_bits(const struct regmap *map, uint8_t reg, uint8_t mask,
                   uint8_t val)
{
	const struct regmap_driver_ops *ops = regmap_ops_for(map);
	uint8_t tmp;
	int err;

	if ((err = ops->read(map, reg, &tmp)))
		return err;

	return ops->write(map, reg, tmp ^ ((val ^ tmp) & mask));
}

int
regmap_device_probe(const struct device *dev)
{
	const struct regmap_device *self = to_regmap_device(dev);

	return regmap_get(&self->map);
}

void
regmap_device_release(const struct device *dev)
{
	const struct regmap_device *self = to_regmap_device(dev);

	regmap_put(&self->map);
}

static inline const struct device *
regmap_to_device(const struct regmap *map)
{
	return &container_of(map, const struct regmap_device, map)->dev;
}

int
regmap_user_probe(const struct regmap *map)
{
	return device_get(regmap_to_device(map));
}

void
regmap_user_release(const struct regmap *map)
{
	device_put(regmap_to_device(map));
}
