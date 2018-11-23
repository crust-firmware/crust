/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int
clock_disable(struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	struct clock_handle *parent;
	struct clock_info   *info = ops->get_info(dev, id);
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
clock_enable(struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	struct clock_handle *parent;
	struct clock_info   *info = ops->get_info(dev, id);
	int err;
	uint32_t rate;

	/* Clamp the rate to the minimum/maximum rates and select a parent. */
	if (!(info->flags & CLK_FIXED)) {
		if ((err = ops->get_rate(dev, id, &rate)))
			return err;
		/* This will fail for fixed-rate or already-enabled clocks. */
		if ((err = clock_set_rate(dev, id, rate)) && err != EPERM)
			return err;
	}

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
clock_get_state(struct device *dev, uint8_t id)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	struct clock_handle *parent;
	struct clock_info   *info = ops->get_info(dev, id);
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

int
clock_set_rate(struct device *dev, uint8_t id, uint32_t rate)
{
	const struct clock_driver_ops *ops = CLOCK_OPS(dev);
	struct clock_info *info = ops->get_info(dev, id);

	/* Prevent changing the rate of clocks that are fixed or are in use. */
	if ((info->flags & CLK_FIXED) || (info->refcount > 0))
		return EPERM;

	/* Call the driver function to change the clock's rate. */
	return ops->set_rate(dev, id, rate);
}
