/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <i2c.h>

int
i2c_probe(struct device *dev)
{
	struct device *i2c_dev         = dev->bus;
	struct i2c_driver_ops *i2c_ops = I2C_OPS(i2c_dev);
	int     err;
	uint8_t dummy;

	assert(i2c_dev);

	/* Start a read transaction. */
	if ((err = i2c_ops->start(i2c_dev, dev->addr, I2C_READ)))
		goto abort;

	/* Read data to avoid putting the slave in an inconsistent state. */
	if (i2c_ops->read(i2c_dev, &dummy))
		goto abort;

abort:
	/* Finish the transaction. */
	i2c_ops->stop(i2c_dev);

	return err;
}

int
i2c_read_reg(struct device *dev, uint8_t addr, uint8_t *data)
{
	struct device *i2c_dev         = dev->bus;
	struct i2c_driver_ops *i2c_ops = I2C_OPS(i2c_dev);
	int err;

	assert(i2c_dev);

	/* Start a write transaction. */
	if ((err = i2c_ops->start(i2c_dev, dev->addr, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = i2c_ops->write(i2c_dev, addr)))
		goto abort;

	/* Restart as a read transaction. */
	if ((err = i2c_ops->start(i2c_dev, dev->addr, I2C_READ)))
		goto abort;

	/* Read the register data. */
	if ((err = i2c_ops->read(i2c_dev, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	i2c_ops->stop(i2c_dev);

	return err;
}

int
i2c_write_reg(struct device *dev, uint8_t addr, uint8_t data)
{
	struct device *i2c_dev         = dev->bus;
	struct i2c_driver_ops *i2c_ops = I2C_OPS(i2c_dev);
	int err;

	assert(i2c_dev);

	/* Start a write transaction. */
	if ((err = i2c_ops->start(i2c_dev, dev->addr, I2C_WRITE)))
		goto abort;

	/* Write the register address. */
	if ((err = i2c_ops->write(i2c_dev, addr)))
		goto abort;

	/* Write the register data. */
	if ((err = i2c_ops->write(i2c_dev, data)))
		goto abort;

abort:
	/* Finish the transaction. */
	i2c_ops->stop(i2c_dev);

	return err;
}
