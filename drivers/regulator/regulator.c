/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <intrusive.h>
#include <regulator.h>
#include <stdbool.h>
#include <stdint.h>

#include "regulator.h"

/**
 * Get the ops for the regulator controller device.
 */
static inline const struct regulator_driver_ops *
regulator_ops_for(const struct device *dev)
{
	const struct regulator_driver *drv =
		container_of(dev->drv, const struct regulator_driver, drv);

	return &drv->ops;
}

int
regulator_disable(const struct device *dev, uint8_t id)
{
	const struct regulator_driver_ops *ops = regulator_ops_for(dev);

	return ops->set_state(dev, id, false);
}

int
regulator_enable(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->set_state(dev, id, true);
}

int
regulator_get_state(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->get_state(dev, id);
}
