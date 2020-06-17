/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <debug.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
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
	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_state_for(clock)->refcount);

	return clock_ops_for(clock)->set_state(clock, CLOCK_STATE_GATED);
}

int
clock_enable(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	int err;

	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_state_for(clock)->refcount);

	/* If the clock has a parent, ensure the parent is enabled. */
	if ((parent = ops->get_parent(clock))) {
		if ((err = clock_enable(parent)))
			return err;
	}

	return ops->set_state(clock, CLOCK_STATE_ENABLED);
}

int
clock_get(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	struct clock_state *state = clock_state_for(clock);
	int err;

	debug("%s: Getting clock %u #%u", clock->dev->name, clock->id,
	      state->refcount + 1);

	/* Perform additional setup if this is the first reference. */
	if (!state->refcount) {
		/* Ensure the controller's driver is loaded. */
		if ((err = device_get(clock->dev)))
			return err;

		/* Ensure the clock's parent has an active reference. */
		if ((parent = ops->get_parent(clock)) &&
		    (err = clock_get(parent))) {
			device_put(clock->dev);
			return err;
		}
	}

	/* Bump the refcount only after successfully acquiring dependencies. */
	++state->refcount;

	/* Enable the clock. */
	if ((err = clock_enable(clock))) {
		clock_put(clock);
		return err;
	}

	debug("%s: Clock %u running at %u Hz", clock->dev->name, clock->id,
	      clock_get_rate(clock));

	return SUCCESS;
}

uint32_t
clock_get_rate(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	uint32_t rate = 0;

	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_state_for(clock)->refcount);

	/* Initialize the rate with the parent's rate or a known safe value. */
	if ((parent = ops->get_parent(clock)))
		rate = clock_get_rate(parent);

	/* Call the driver function to adjust this clock's rate. */
	return ops->get_rate(clock, rate);
}

int
clock_get_state(const struct clock_handle *clock, int *state)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	int err;

	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_state_for(clock)->refcount);

	/* If the clock has a parent, check the parent's state. */
	if ((parent = ops->get_parent(clock))) {
		if ((err = clock_get_state(parent, state)))
			return err;

		/* If the parent is not enabled, this clock has that state. */
		if (*state != CLOCK_STATE_ENABLED)
			return SUCCESS;
	}

	/* Call the driver function to read any gate this clock may have. */
	return ops->get_state(clock, state);
}

void
clock_put(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	struct clock_state *state = clock_state_for(clock);
	int err;

	debug("%s: Putting clock %u #%u", clock->dev->name, clock->id,
	      state->refcount);

	/* Calling this function is only allowed after calling clock_get(). */
	assert(state->refcount);

	/* Do nothing if there are other consumers of this clock. */
	if (--state->refcount)
		return;

	/* Completely disable the clock once the last consumer is gone. */
	if ((err = ops->set_state(clock, CLOCK_STATE_DISABLED))) {
		debug("%s: Clock %u release failed",
		      clock->dev->name, clock->id);
	}

	/* Drop the reference to the parent clock. */
	if ((parent = ops->get_parent(clock)))
		clock_put(parent);

	/* Drop the reference to the controller device. */
	device_put(clock->dev);
}
