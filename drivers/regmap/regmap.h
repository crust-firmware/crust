/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef REGMAP_PRIVATE_H
#define REGMAP_PRIVATE_H

#include <device.h>
#include <regmap.h>
#include <stdint.h>

struct regmap_driver_ops {
	int (*prepare)(const struct regmap *map);
	int (*read)(const struct regmap *map, uint8_t reg, uint8_t *val);
	int (*write)(const struct regmap *map, uint8_t reg, uint8_t val);
};

struct regmap_driver {
	struct driver            drv;
	struct regmap_driver_ops ops;
};

#endif /* REGMAP_PRIVATE_H */
