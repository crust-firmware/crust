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
	return container_of(dev, struct sunxi_ccu, dev);
}

static struct sunxi_ccu_clock *
get_clock(const struct sunxi_ccu *self, uint8_t id)
{
	return &self->clocks[id];
}

static struct clock_info *
sunxi_ccu_get_info(const struct device *dev, uint8_t id)
{
	const struct sunxi_ccu *self = to_sunxi_ccu(dev);

	return &get_clock(self, id)->info;
}

static const struct clock_handle *
sunxi_ccu_get_parent(const struct device *dev, uint8_t id)
{
	const struct sunxi_ccu *self  = to_sunxi_ccu(dev);
	struct sunxi_ccu_clock *clock = get_clock(self, id);
	size_t index = 0;

	if (BF_PRESENT(clock->mux)) {
		uint32_t reg = mmio_read_32(self->regs + clock->reg);
		index = bitfield_get(reg, clock->mux);
	}

	return &clock->parents[index];
}

static int
sunxi_ccu_get_rate(const struct device *dev, uint8_t id, uint32_t *rate)
{
	const struct clock_handle *parent = sunxi_ccu_get_parent(dev, id);
	const struct sunxi_ccu *self = to_sunxi_ccu(dev);
	struct sunxi_ccu_clock *clock = get_clock(self, id);
	uint32_t reg, tmp;
	int err;

	/* If a clock has no parent, it runs at a fixed rate. Return that. */
	if (parent == NULL) {
		*rate = clock->info.max_rate;
		return SUCCESS;
	}

	/* Otherwise, the rate is the parent's rate divided by some factors. */
	if ((err = clock_get_rate(parent->dev, parent->id, &tmp)))
		return err;
	reg   = mmio_read_32(self->regs + clock->reg);
	tmp  /= bitfield_get(reg, parent->vdiv) + 1;
	tmp  /= bitfield_get(reg, clock->m) + 1;
	tmp >>= bitfield_get(reg, clock->p);
	*rate = tmp;

	return SUCCESS;
}

static int
sunxi_ccu_get_state(const struct device *dev, uint8_t id)
{
	const struct sunxi_ccu *self  = to_sunxi_ccu(dev);
	struct sunxi_ccu_clock *clock = get_clock(self, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Check the bus clock gate. */
	if (gate != 0 && !bitmap_get(self->regs, gate))
		return false;
	/* Check the reset line. */
	if (reset != 0 && !bitmap_get(self->regs, reset))
		return false;

	return true;
}

static int
sunxi_ccu_set_state(const struct device *dev, uint8_t id, bool enable)
{
	const struct sunxi_ccu *self  = to_sunxi_ccu(dev);
	struct sunxi_ccu_clock *clock = get_clock(self, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

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
		.get_info   = sunxi_ccu_get_info,
		.get_parent = sunxi_ccu_get_parent,
		.get_rate   = sunxi_ccu_get_rate,
		.get_state  = sunxi_ccu_get_state,
		.set_state  = sunxi_ccu_set_state,
	},
};
