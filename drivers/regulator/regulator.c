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
regulator_set_state(const struct regulator_handle *handle, bool enable)
{
	int err;

	if ((err = device_get(handle->dev)))
		return err;

	err = regulator_ops_for(handle->dev)->set_state(handle, enable);

	device_put(handle->dev);

	return err;
}

int
regulator_disable(const struct regulator_handle *handle)
{
	return regulator_set_state(handle, false);
}

int
regulator_enable(const struct regulator_handle *handle)
{
	return regulator_set_state(handle, true);
}

int
regulator_get_state(const struct regulator_handle *handle)
{
	int err;

	if ((err = device_get(handle->dev)))
		return err;

	err = regulator_ops_for(handle->dev)->get_state(handle);

	device_put(handle->dev);

	return err;
}
