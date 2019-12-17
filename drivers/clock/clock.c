/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int
clock_disable(const struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	const struct clock_handle *parent;
	struct clock_info *info = ops->get_info(dev, id);
	int err;

	/* Prevent disabling clocks that are critical or in use as parents. */
	if ((info->flags & CLK_CRITICAL) || (info->refcount > 1)) {
		info->refcount--;
		return EPERM;
	}

	/* Call the driver function to change the clock's state. */
	if ((err = ops->set_state(dev, id, false)))
		return err;

	/* Mark the clock and its parent as no longer being in use. */
	info->refcount--;
	if ((parent = ops->get_parent(dev, id)) != NULL)
		clock_get_info(parent->dev, parent->id)->refcount--;

	return SUCCESS;
}

int
clock_enable(const struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	const struct clock_handle *parent;
	struct clock_info *info = ops->get_info(dev, id);
	int err;

	/* Enable the parent clock, if it exists, and increase its refcount. */
	if ((parent = ops->get_parent(dev, id)) != NULL) {
		if ((err = clock_enable(parent->dev, parent->id)))
			return err;
	}

	/* Call the driver function to change the clock's state. */
	if ((err = ops->set_state(dev, id, true)))
		return err;

	/* Mark the clock itself as being in use. */
	info->refcount++;

	return SUCCESS;
}

int
clock_get(const struct clock_handle *clock)
{
	const struct device *dev = device_get(clock->dev);
	uint8_t id = clock->id;
	int err;

	/* Ensure the controller's driver is loaded. */
	if (!dev)
		return ENODEV;

	/* Enable the clock. */
	if ((err = clock_enable(dev, id)))
		return err;

	return SUCCESS;
}

int
clock_get_state(const struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	const struct clock_handle *parent;
	struct clock_info *info = ops->get_info(dev, id);
	int err;

	/* If this clock is in use, it must have been enabled. */
	if (info->refcount > 0)
		return true;

	/* If the parent clock is disabled, this clock cannot be enabled. */
	if ((parent = ops->get_parent(dev, id)) != NULL) {
		/* Propagate any error or a false return value. */
		if ((err = clock_get_state(parent->dev, parent->id)) < true)
			return err;
	}

	/* Call the driver function to read any gate this clock may have. */
	return ops->get_state(dev, id);
}
