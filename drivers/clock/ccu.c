/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <error.h>
#include <stdbool.h>
#include <clock/ccu.h>

#include "ccu.h"

const struct clock_handle *
ccu_get_parent(const struct clock_handle *clock)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	return clk->get_parent(self, clk);
}

uint32_t
ccu_get_rate(const struct clock_handle *clock, uint32_t rate)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];

	/* Perform clock-specific adjustments to the parent rate. */
	return clk->get_rate(self, clk, rate);
}

int
ccu_get_state(const struct clock_handle *clock, int *state)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	uintptr_t regs = self->regs;

	/* Check the reset line, if present. */
	if (clk->reset && !bitmap_get(regs, clk->reset))
		*state = CLOCK_STATE_DISABLED;
	/* Check the clock gate, if present. */
	else if (clk->gate && !bitmap_get(regs, clk->gate))
		*state = CLOCK_STATE_GATED;
	/* Otherwise, the clock is enabled. */
	else
		*state = CLOCK_STATE_ENABLED;

	return SUCCESS;
}

int
ccu_set_state(const struct clock_handle *clock, int state)
{
	const struct ccu *self      = to_ccu(clock->dev);
	const struct ccu_clock *clk = &self->clocks[clock->id];
	bool enable    = state > CLOCK_STATE_DISABLED;
	bool ungate    = state > CLOCK_STATE_GATED;
	uintptr_t regs = self->regs;

	/* Ungate the clock before taking the device out of reset. */
	if (clk->gate && ungate)
		bitmap_set(regs, clk->gate);
	/* Assert or deassert the reset line while the clock is running. */
	if (clk->reset)
		(enable ? bitmap_set : bitmap_clear)(regs, clk->reset);
	/* Gate the clock after putting the device in reset. */
	if (clk->gate && !ungate)
		bitmap_clear(regs, clk->gate);
	/* Apply the changes by setting the update bit, if applicable. */
	if (clk->update)
		mmio_set_32(regs + clk->reg, BIT(clk->update));
	/* Wait for the lock bit to be set, if applicable. */
	if (clk->lock && ungate)
		mmio_poll_32(regs + clk->reg, BIT(clk->lock));

	return SUCCESS;
}
