/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
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

bool
clock_active(const struct clock_handle *clock)
{
	return clock_state_for(clock)->refcount;
}

void
clock_disable(const struct clock_handle *clock)
{
	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_active(clock));

	clock_ops_for(clock)->set_state(clock, CLOCK_STATE_GATED);
}

void
clock_enable(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;

	/* Calling this function is only allowed after calling clock_get(). */
	assert(clock_active(clock));

	/* If the clock has a parent, ensure the parent is enabled. */
	if ((parent = ops->get_parent(clock)))
		clock_enable(parent);

	ops->set_state(clock, CLOCK_STATE_ENABLED);
}

void
clock_get(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	struct clock_state *state = clock_state_for(clock);

	/* Perform additional setup if this is the first reference. */
	if (!state->refcount) {
		const struct clock_handle *parent;
		int err UNUSED;

		/* Ensure the controller's driver is loaded. */
		err = device_get(clock->dev);
		assert(err == SUCCESS);

		/* Ensure the clock's parent has an active reference. */
		if ((parent = ops->get_parent(clock)))
			clock_get(parent);

		debug("%s: Clock %u running at %u Hz", clock->dev->name,
		      clock->id, clock_get_rate(clock));
	}

	/* Bump the refcount only after successfully acquiring dependencies. */
	++state->refcount;

	/* Enable the clock. */
	clock_enable(clock);
}

uint32_t
clock_get_rate(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	uint32_t rate = 0;

	/* Initialize the rate with the parent's rate or a known safe value. */
	if ((parent = ops->get_parent(clock)))
		rate = clock_get_rate(parent);

	/* Call the driver function to calculate this clock's rate. */
	return ops->get_rate(clock, rate);
}

uint32_t
clock_get_state(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	uint32_t parent_state;

	/* If the clock has a parent, check the parent's state. */
	if ((parent = ops->get_parent(clock))) {
		parent_state = clock_get_state(parent);

		/* If the parent is not enabled, this clock has that state. */
		if (parent_state != CLOCK_STATE_ENABLED)
			return parent_state;
	}

	/* Call the driver function to check this clock's state. */
	return ops->get_state(clock);
}

void
clock_put(const struct clock_handle *clock)
{
	const struct clock_driver_ops *ops = clock_ops_for(clock);
	const struct clock_handle *parent;
	struct clock_state *state = clock_state_for(clock);

	/* Calling this function is only allowed after calling clock_get(). */
	assert(state->refcount);

	/* Do nothing if there are other consumers of this clock. */
	if (--state->refcount)
		return;

	debug("%s: Releasing clock %u", clock->dev->name, clock->id);

	/* Completely disable the clock once the last consumer is gone. */
	ops->set_state(clock, CLOCK_STATE_DISABLED);

	/* Drop the reference to the parent clock. */
	if ((parent = ops->get_parent(clock)))
		clock_put(parent);

	/* Drop the reference to the controller device. */
	device_put(clock->dev);
}
