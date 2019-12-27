/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stddef.h>
#include <clock/ccu.h>

#include "ccu.h"

static inline const struct ccu *
to_ccu(const struct device *dev)
{
	return container_of(dev, const struct ccu, dev);
}

static const struct clock_handle *
ccu_get_parent(const struct clock_handle *clock)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	size_t index = 0;

	if (BF_PRESENT(clk->mux)) {
		uint32_t reg = mmio_read_32(self->regs + clk->reg);
		index = bitfield_get(reg, BF_OFFSET(clk->mux),
		                     BF_WIDTH(clk->mux));
	}

	return &clk->parents[index];
}

static uint32_t
ccu_get_rate(const struct clock_handle *clock, uint32_t rate)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	/* Perform clock-specific adjustments to the parent rate. */
	return clk->get_rate(self, rate, clock->id);
}

static int
ccu_get_state(const struct clock_handle *clock, int *state)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	/* Check the reset line, if present. */
	if (clk->reset && !bitmap_get(self->regs, clk->reset))
		*state = CLOCK_STATE_DISABLED;
	/* Check the clock gate, if present. */
	else if (clk->gate && !bitmap_get(self->regs, clk->gate))
		*state = CLOCK_STATE_GATED;
	/* Otherwise, the clock is enabled. */
	else
		*state = CLOCK_STATE_ENABLED;

	return SUCCESS;
}

static int
ccu_set_state(const struct clock_handle *clock, int state)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	bool enable = state > CLOCK_STATE_DISABLED;
	bool ungate = state > CLOCK_STATE_GATED;

	/* Ungate the clock before taking the device out of reset. */
	if (clk->gate && ungate)
		bitmap_set(self->regs, clk->gate);
	/* Assert or deassert the reset line while the clock is running. */
	if (clk->reset)
		(enable ? bitmap_set : bitmap_clear)(self->regs, clk->reset);
	/* Gate the clock after putting the device in reset. */
	if (clk->gate && !ungate)
		bitmap_clear(self->regs, clk->gate);

	/* Verify the registers match the expected final values. */
	if (clk->gate && bitmap_get(self->regs, clk->gate) != ungate)
		return EIO;
	if (clk->reset && bitmap_get(self->regs, clk->reset) != enable)
		return EIO;

	return SUCCESS;
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
