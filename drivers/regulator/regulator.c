/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
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

static int
regulator_bulk_set(const struct regulator_list *list, bool enable)
{
	const struct regulator_driver_ops *ops = regulator_ops_for(list->dev);
	int err, ret = SUCCESS;

	if ((err = device_get(list->dev)))
		return err;

	for (uint8_t i = 0; i < list->nr_ids; ++i) {
		err = ops->set_state(list->dev, list->ids[i], enable);
		if (err && !ret)
			ret = err;
	}

	device_put(list->dev);

	return ret;
}

int
regulator_bulk_disable(const struct regulator_list *list)
{
	return regulator_bulk_set(list, false);
}

int
regulator_bulk_enable(const struct regulator_list *list)
{
	return regulator_bulk_set(list, true);
}

int
regulator_get_state(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->get_state(dev, id);
}
