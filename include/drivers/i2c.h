/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_I2C_H
#define DRIVERS_I2C_H

#include <dm.h>
#include <stdint.h>

#define I2C_OPS(dev) ((struct i2c_driver_ops *)((dev)->drv->ops))

struct i2c_driver_ops {
	int (*read)(struct device *dev, uint8_t *data);
	int (*start)(struct device *dev, uint8_t addr, uint8_t direction);
	int (*stop)(struct device *dev);
	int (*write)(struct device *dev, uint8_t data);
};

enum {
	I2C_READ  = 1,
	I2C_WRITE = 0,
};

/**
 * Probe an I2C device.
 *
 * @param dev  The I2C device to probe.
 */
int i2c_probe(struct device *dev);

/**
 * Read the register of an I2C device.
 *
 * @param dev   The I2C device that contains the register to read.
 * @param addr  The register address of the I2C device.
 * @param data  The location to save the contents read from the I2C device
 * register.
 */
int i2c_read_reg(struct device *dev, uint8_t addr, uint8_t *data);

/**
 * Write to the register of an I2C device.
 *
 * @param dev   The I2C device that contains the register to write to.
 * @param addr  The register address of the I2C device.
 * @param data  The data to write to the register of the supplied I2C device.
 */
int i2c_write_reg(struct device *dev, uint8_t addr, uint8_t data);

#endif /* DRIVERS_I2C_H */
