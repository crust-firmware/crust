/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <i2c.h>
#include <stdint.h>

int
i2c_probe(struct device *dev, uint8_t addr)
{
	const struct i2c_driver_ops *ops = I2C_OPS(dev);
	int     err;
	uint8_t dummy;

	/* Start a read transaction. */
	if ((err = ops->start(dev, addr, I2C_READ)))
		goto abort;

	/* Read data to avoid putting the device in an inconsistent state. */
	if (ops->read(dev, &dummy))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(dev);

	return err;
}

int
i2c_read_reg(struct device *dev, uint8_t addr, uint8_t reg, uint8_t *data)
{
	const struct i2c_driver_ops *ops = I2C_OPS(dev);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(dev, addr, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(dev, reg)))
		goto abort;

	/* Restart as a read transaction. */
	if ((err = ops->start(dev, addr, I2C_READ)))
		goto abort;

	/* Read the register data. */
	if ((err = ops->read(dev, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(dev);

	return err;
}

int
i2c_write_reg(struct device *dev, uint8_t addr, uint8_t reg, uint8_t data)
{
	const struct i2c_driver_ops *ops = I2C_OPS(dev);
	int err;

	/* Start a write transaction. */
	if ((err = ops->start(dev, addr, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = ops->write(dev, reg)))
		goto abort;

	/* Write the register data. */
	if ((err = ops->write(dev, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	ops->stop(dev);

	return err;
}
