/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_I2C_H
#define DRIVERS_I2C_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>

#define I2C_OPS(dev) \
	(&container_of((dev)->drv, struct i2c_driver, drv)->ops)

#define I2C_NUM_PINS 2

enum {
	I2C_READ  = 1,
	I2C_WRITE = 0,
};

struct i2c_driver_ops {
	int  (*read)(struct device *dev, uint8_t *data);
	int  (*start)(struct device *dev, uint8_t addr, uint8_t direction);
	void (*stop)(struct device *dev);
	int  (*write)(struct device *dev, uint8_t data);
};

struct i2c_driver {
	const struct driver         drv;
	const struct i2c_driver_ops ops;
};

/**
 * Probe for the existence of an I²C device.
 *
 * @param dev  The I²C controller that the device is connected to.
 * @param addr The address of the I²C device on the bus.
 */
int i2c_probe(struct device *dev, uint8_t addr);

/**
 * Read a register contained inside an I²C device.
 *
 * @param dev   The I²C controller that the device is connected to.
 * @param addr  The address of the I²C device on the bus.
 * @param reg   The register within the the I²C device to read.
 * @param data  The location to save the data read from the register.
 */
int i2c_read_reg(struct device *dev, uint8_t addr, uint8_t reg, uint8_t *data);

/**
 * Write to a register contained inside an I²C device.
 *
 * @param dev   The I²C controller that the device is connected to.
 * @param addr  The address of the I²C device on the bus.
 * @param reg   The register within the the I²C device to write.
 * @param data  The data to write to the register.
 */
int i2c_write_reg(struct device *dev, uint8_t addr, uint8_t reg, uint8_t data);

#endif /* DRIVERS_I2C_H */
