/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef REGMAP_I2C_PRIVATE_H
#define REGMAP_I2C_PRIVATE_H

#include "regmap.h"

enum {
	I2C_WRITE = 0,
	I2C_READ  = 1,
};

struct regmap_i2c_driver_ops {
	int  (*read)(const struct regmap *map, uint8_t *data);
	int  (*start)(const struct regmap *map, uint8_t direction);
	void (*stop)(const struct regmap *map);
	int  (*write)(const struct regmap *map, uint8_t data);
};

struct regmap_i2c_driver {
	struct regmap_driver         drv;
	struct regmap_i2c_driver_ops ops;
};

int regmap_i2c_prepare(const struct regmap *map);

int regmap_i2c_read(const struct regmap *map, uint8_t reg, uint8_t *val);

int regmap_i2c_write(const struct regmap *map, uint8_t reg, uint8_t val);

#endif /* REGMAP_I2C_PRIVATE_H */
