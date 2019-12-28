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
 * Get a reference to an I²C device and its bus controller.
 *
 * This function will probe for the existence of the I²C device.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   ENODEV The device was not found on the I²C bus.
 *
 * @param bus   A handle specifying the I²C controller and device address.
 * @return      Zero on success; an error code on failure.
 */
int i2c_get(const struct i2c_handle *bus);

/**
 * Release a reference to an I²C device and its bus controller.
 *
 * @param bus   A reference to an I²C device.
 */
void i2c_put(const struct i2c_handle *bus);

/**
 * Read a register contained inside an I²C device.
 *
 * @param bus   A reference to an I²C device.
 * @param addr  The register within the the I²C device to read.
 * @param data  The location to save the data read from the register.
 */
int i2c_read_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t *data);

/**
 * Write to a register contained inside an I²C device.
 *
 * @param bus   A reference to an I²C device.
 * @param addr  The register within the the I²C device to write.
 * @param data  The data to write to the register.
 */
int i2c_write_reg(const struct i2c_handle *bus, uint8_t addr, uint8_t data);

#endif /* DRIVERS_I2C_H */
