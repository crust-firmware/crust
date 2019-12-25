/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <error.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "clock.h"

/**
 * Get the ops for the controller device providing this clock.
 */
static inline const struct clock_driver_ops *
clock_ops_for(const struct clock_handle *clock)
{
	const struct clock_driver *drv =
		container_of(clock->dev->drv, const struct clock_driver, drv);

	return &drv->ops;
}

/**
 * Get the mutable state for this clock.
 */
static inline struct clock_state *
clock_state_for(const struct clock_handle *clock)
{
	struct clock_device_state *state =
		container_of(clock->dev->state, struct clock_device_state, ds);

	return &state->cs[clock->id];
}

int
clock_disable(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	struct clock_info  *info  = ops->get_info(clock);
	struct clock_state *state = clock_state_for(clock);
	int err;

	/* Prevent disabling clocks that are critical or in use as parents. */
	if ((info->flags & CLK_CRITICAL) || (state->refcount > 1)) {
		state->refcount--;
		return EPERM;
	}

	/* Call the driver function to change the clock's state. */
	if ((err = ops->set_state(clock, false)))
		return err;

	/* Mark the clock and its parent as no longer being in use. */
	state->refcount--;
	if ((parent = ops->get_parent(clock)) != NULL)
		clock_state_for(parent)->refcount--;

	return SUCCESS;
}

int
clock_enable(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	int err;

	/* Enable the parent clock, if it exists, and increase its refcount. */
	if ((parent = ops->get_parent(clock))) {
		if ((err = clock_enable(parent)))
			return err;
	}

	/* Call the driver function to change the clock's state. */
	if ((err = ops->set_state(clock, true)))
		return err;

	/* Mark the clock itself as being in use. */
	clock_state_for(clock)->refcount++;

	return SUCCESS;
}

int
clock_get(const struct clock_handle *clock)
{
	int err;

	/* Ensure the controller's driver is loaded. */
	if (!device_get(clock->dev))
		return ENODEV;

	/* Enable the clock. */
	if ((err = clock_enable(clock)))
		return err;

	return SUCCESS;
}

int
clock_get_rate(const struct clock_handle *clock, uint32_t *rate)
{
	return clock_ops_for(clock)->get_rate(clock, rate);
}

int
clock_get_state(const struct clock_handle *clock, bool *state)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	int err;

	/* If this clock is in use, it must have been enabled. */
	if (clock_state_for(clock)->refcount > 0) {
		*state = true;
		return SUCCESS;
	}

	/* If the clock has a parent, check the parent's state. */
	if ((parent = ops->get_parent(clock))) {
		if ((err = clock_get_state(parent, state)))
			return err;

		/* If the parent is not enabled, this clock is not either. */
		if (*state != true)
			return SUCCESS;
	}

	/* Call the driver function to read any gate this clock may have. */
	return ops->get_state(clock, state);
}
