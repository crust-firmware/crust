/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <i2c.h>
#include <intrusive.h>
#include <stdint.h>

#include "i2c.h"

/**
 * Get the ops for the I²C controller device.
 */
static inline const struct i2c_driver_ops *
i2c_ops_for(const struct i2c_handle *bus)
{
	const struct i2c_driver *drv =
		container_of(bus->dev->drv, const struct i2c_driver, drv);

	return &drv->ops;
}

int
i2c_probe(const struct i2c_handle *bus)
{
	const struct i2c_driver_ops *ops = i2c_ops_for(bus);
	uint8_t dummy;
	int err;

	/* Ensure the controller's driver is loaded. */
	if (!device_get(bus->dev))
		return ENODEV;

	/* Start a read transaction. */
	if ((err = ops->start(bus, I2C_READ)))
		goto abort;

	/* Read data to avoid putting the device in an inconsistent state. */
	if (ops->read(bus, &dummy))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(bus);

	return err;
}

int
i2c_read_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t *data)
{
	const struct i2c_driver_ops *ops = i2c_ops_for(bus);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(bus, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(bus, addr)))
		goto abort;

	/* Restart as a read transaction. */
	if ((err = ops->start(bus, I2C_READ)))
		goto abort;

	/* Read the register data. */
	if ((err = ops->read(bus, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(bus);

	return err;
}

int
i2c_write_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t data)
{
	const struct i2c_driver_ops *ops = i2c_ops_for(bus);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(bus, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(bus, addr)))
		goto abort;

	/* Write the register data. */
	if ((err = ops->write(bus, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(bus);

	return err;
}
