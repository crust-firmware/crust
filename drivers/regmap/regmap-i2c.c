/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <stdint.h>

#include "regmap-i2c.h"

/**
 * Get the ops for the I²C controller device.
 */
static inline const struct regmap_i2c_driver_ops *
regmap_i2c_ops_for(const struct regmap *map)
{
	const struct regmap_i2c_driver *drv =
		container_of(map->dev->drv, const struct regmap_i2c_driver,
		             drv.drv);

	return &drv->ops;
}

int
regmap_i2c_prepare(const struct regmap *map)
{
	const struct regmap_i2c_driver_ops *ops = regmap_i2c_ops_for(map);
	uint8_t dummy;
	int err;

	/* Start a read transaction. */
	if ((err = ops->start(map, I2C_READ)))
		goto abort;

	/* Read data to avoid putting the device in an inconsistent state. */
	if (ops->read(map, &dummy))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(map);

	return err ? ENODEV : SUCCESS;
}

int
regmap_i2c_read(const struct regmap *map, uint8_t reg, uint8_t *val)
{
	const struct regmap_i2c_driver_ops *ops = regmap_i2c_ops_for(map);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(map, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(map, reg)))
		goto abort;

	/* Restart as a read transaction. */
	if ((err = ops->start(map, I2C_READ)))
		goto abort;

	/* Read the register value. */
	if ((err = ops->read(map, val)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(map);

	return err;
}

int
regmap_i2c_write(const struct regmap *map, uint8_t reg, uint8_t val)
{
	const struct regmap_i2c_driver_ops *ops = regmap_i2c_ops_for(map);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(map, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(map, reg)))
		goto abort;

	/* Write the register value. */
	if ((err = ops->write(map, val)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(map);

	return err;
}
