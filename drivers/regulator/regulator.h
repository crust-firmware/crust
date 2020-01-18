/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef REGULATOR_PRIVATE_H
#define REGULATOR_PRIVATE_H

#include <device.h>
#include <regulator.h>
#include <stdint.h>

struct regulator_driver_ops {
	const struct regulator_info *
	    (*get_info)(const struct device *dev, uint8_t id);
	int (*get_state)(const struct device *dev, uint8_t id);
	int (*read_raw)(const struct device *dev, uint8_t id, uint32_t *raw);
	int (*set_state)(const struct device *dev, uint8_t id, bool enable);
	int (*write_raw)(const struct device *dev, uint8_t id, uint32_t raw);
};

struct regulator_driver {
	struct driver               drv;
	struct regulator_driver_ops ops;
};

#endif /* REGULATOR_PRIVATE_H */
