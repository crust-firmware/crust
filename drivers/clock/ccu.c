/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <stdbool.h>
#include <clock/ccu.h>

#include "ccu.h"

static inline const struct ccu *
to_ccu(const struct device *dev)
{
	return container_of(dev, const struct ccu, dev);
}

const struct clock_handle *
ccu_get_null_parent(const struct ccu *self UNUSED,
                    const struct ccu_clock *clk UNUSED)
{
	return NULL;
}

static const struct clock_handle *
ccu_get_parent(const struct clock_handle *clock)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	return clk->get_parent(self, clk);
}

uint32_t
ccu_get_parent_rate(const struct ccu *self UNUSED,
                    const struct ccu_clock *clk UNUSED, uint32_t rate)
{
	return rate;
}

static uint32_t
ccu_get_rate(const struct clock_handle *clock, uint32_t rate)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	/* Perform clock-specific adjustments to the parent rate. */
	return clk->get_rate(self, clk, rate);
}

static uint32_t
ccu_get_state(const struct clock_handle *clock)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	uintptr_t regs = self->regs;

	/* Check the reset line, if present. */
	if (clk->reset && !bitmap_get(regs, clk->reset))
		return CLOCK_STATE_DISABLED;
	/* Check the clock gate, if present. */
	if (clk->gate && !bitmap_get(regs, clk->gate))
		return CLOCK_STATE_GATED;

	/* Otherwise, the clock is enabled. */
	return CLOCK_STATE_ENABLED;
}

static void
ccu_set_state(const struct clock_handle *clock, uint32_t state)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	bool enable    = state > CLOCK_STATE_DISABLED;
	bool ungate    = state > CLOCK_STATE_GATED;
	uintptr_t regs = self->regs;

	/* Do nothing if the clock is already in the desired state. */
	if (ccu_get_state(clock) == state)
		return;

	/* First, (de)assert the reset line. */
	if (clk->reset)
		(enable ? bitmap_set : bitmap_clear)(regs, clk->reset);
	/* Once the device is in/out of reset, (un)gate the clock. */
	if (clk->gate)
		(ungate ? bitmap_set : bitmap_clear)(regs, clk->gate);
	/* Apply the changes by setting the update bit, if applicable. */
	if (clk->update)
		mmio_set_32(regs + clk->reg, BIT(clk->update));
	/* Wait for the lock bit to be set, if applicable. */
	if (clk->lock && ungate)
		mmio_poll_32(regs + clk->reg, BIT(clk->lock));
}

const struct clock_driver ccu_driver = {
	.drv = {
		.probe   = dummy_probe,
		.release = dummy_release,
	},
	.ops = {
		.get_parent = ccu_get_parent,
		.get_rate   = ccu_get_rate,
		.get_state  = ccu_get_state,
		.set_state  = ccu_set_state,
	},
};
