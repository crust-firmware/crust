/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef REGULATOR_PRIVATE_H
#define REGULATOR_PRIVATE_H

#include <device.h>
#include <stdbool.h>
#include <stdint.h>

struct regulator_driver_ops {
	int (*get_state)(const struct device *dev, uint8_t id);
	int (*set_state)(const struct device *dev, uint8_t id, bool enable);
};

struct regulator_driver {
	struct driver               drv;
	struct regulator_driver_ops ops;
};

#endif /* REGULATOR_PRIVATE_H */
