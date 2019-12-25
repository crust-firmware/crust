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
#include <clock/sunxi-ccu.h>

#include "sunxi-ccu.h"

static inline const struct sunxi_ccu *
to_sunxi_ccu(const struct device *dev)
{
	return container_of(dev, const struct sunxi_ccu, dev);
}

static const struct clock_handle *
sunxi_ccu_get_parent(const struct clock_handle *clock)
{
	const struct sunxi_ccu *self      = to_sunxi_ccu(clock->dev);
	const struct sunxi_ccu_clock *clk = &self->clocks[clock->id];
	size_t index = 0;

	if (BF_PRESENT(clk->mux)) {
		uint32_t reg = mmio_read_32(self->regs + clk->reg);
		index = bitfield_get(reg, clk->mux);
	}

	return &clk->parents[index];
}

static int
sunxi_ccu_get_rate(const struct clock_handle *clock, uint32_t *rate)
{
	const struct sunxi_ccu *self = to_sunxi_ccu(clock->dev);
	const struct sunxi_ccu_clock *clk = &self->clocks[clock->id];
	uint32_t reg, tmp;
	int err;

	/* Perform clock-specific adjustments to the parent rate. */
	if (clk->get_rate && (err = clk->get_rate(self, clock->id, rate)))
		return err;

	/* Apply the standard dividers to the clock rate. */
	reg   = mmio_read_32(self->regs + clk->reg);
	tmp   = *rate;
	tmp  /= bitfield_get(reg, clk->m) + 1;
	tmp >>= bitfield_get(reg, clk->p);
	*rate = tmp;

	return SUCCESS;
}

static int
sunxi_ccu_get_state(const struct clock_handle *clock, bool *state)
{
	const struct sunxi_ccu *self      = to_sunxi_ccu(clock->dev);
	const struct sunxi_ccu_clock *clk = &self->clocks[clock->id];
	uint16_t gate  = clk->gate;
	uint16_t reset = clk->reset;

	/* Check the bus clock gate. */
	if (gate != 0 && !bitmap_get(self->regs, gate))
		*state = false;
	/* Check the reset line. */
	else if (reset != 0 && !bitmap_get(self->regs, reset))
		*state = false;
	else
		*state = true;

	return SUCCESS;
}

static int
sunxi_ccu_set_state(const struct clock_handle *clock, bool enable)
{
	const struct sunxi_ccu *self      = to_sunxi_ccu(clock->dev);
	const struct sunxi_ccu_clock *clk = &self->clocks[clock->id];
	uint16_t gate  = clk->gate;
	uint16_t reset = clk->reset;

	if (enable) {
		/* Enable the clock before taking the device out of reset. */
		if (gate != 0) {
			bitmap_set(self->regs, gate);
			if (!bitmap_get(self->regs, gate))
				return EIO;
		}
		/* Deassert the reset once the device has a running clock. */
		if (reset != 0) {
			bitmap_set(self->regs, reset);
			if (!bitmap_get(self->regs, reset))
				return EIO;
		}
	} else {
		/* Put the device in reset before turning off its clock. */
		if (reset != 0) {
			bitmap_clear(self->regs, reset);
			if (bitmap_get(self->regs, reset))
				return EIO;
		}
		/* Finally gate the bus clock. */
		if (gate != 0) {
			bitmap_clear(self->regs, gate);
			if (bitmap_get(self->regs, gate))
				return EIO;
		}
	}

	return SUCCESS;
}

const struct clock_driver sunxi_ccu_driver = {
	.drv = {
		.probe   = dummy_probe,
		.release = dummy_release,
	},
	.ops = {
		.get_parent = sunxi_ccu_get_parent,
		.get_rate   = sunxi_ccu_get_rate,
		.get_state  = sunxi_ccu_get_state,
		.set_state  = sunxi_ccu_set_state,
	},
};
