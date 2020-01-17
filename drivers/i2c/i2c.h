/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef I2C_PRIVATE_H
#define I2C_PRIVATE_H

#include <device.h>
#include <i2c.h>
#include <stdint.h>

enum {
	I2C_WRITE = 0,
	I2C_READ  = 1,
};

struct i2c_driver_ops {
	int  (*read)(const struct i2c_handle *bus, uint8_t *data);
	int  (*start)(const struct i2c_handle *bus, uint8_t direction);
	void (*stop)(const struct i2c_handle *bus);
	int  (*write)(const struct i2c_handle *bus, uint8_t data);
};

struct i2c_driver {
	struct driver         drv;
	struct i2c_driver_ops ops;
};

#endif /* I2C_PRIVATE_H */
