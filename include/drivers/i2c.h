/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_I2C_H
#define DRIVERS_I2C_H

#include <device.h>
#include <stdint.h>

struct i2c_handle {
	const struct device *dev;
	uint8_t              id;
};

/**
 * Probe for the existence of an I²C device.
 *
 * @param bus   A reference to the I²C controller and device address.
 */
int i2c_probe(const struct i2c_handle *bus);

/**
 * Read a register contained inside an I²C device.
 *
 * @param bus   A reference to the I²C controller and device address.
 * @param addr  The register within the the I²C device to read.
 * @param data  The location to save the data read from the register.
 */
int i2c_read_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t *data);

/**
 * Write to a register contained inside an I²C device.
 *
 * @param bus   A reference to the I²C controller and device address.
 * @param addr  The register within the the I²C device to write.
 * @param data  The data to write to the register.
 */
int i2c_write_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t data);

#endif /* DRIVERS_I2C_H */
