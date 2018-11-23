/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <compiler.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
#include <error.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <clock/sunxi-ccu.h>

struct sunxi_ccu_factors {
	uint8_t mux; /**< Mux index for the parent. */
	uint8_t pd;  /**< Linear parent PD ("post-divider"). */
	uint8_t m;   /**< Linear M ("multiple") divider. */
	uint8_t p;   /**< Exponential P ("power") divider. */
};

static struct sunxi_ccu_clock *
get_clock(struct device *dev, uint8_t id)
{
	return &((struct sunxi_ccu_clock *)dev->drvdata)[id];
}

/*
 * Choose the parent and set of dividers that will configure the clock to run
 * as close as possible to the desired rate.
 *
 * Iterate through each possible parent and each combination of dividers
 * (including a pre-divider if present for that parent) until an exact match
 * is found. If no exact match is found, return the best match that is between
 * this clock's minimum and maximum rates.
 *
 * Order the iterations to favor changing PD divider over the M divider (since
 * use of the M divider is sometimes recommended against, and the PD and M
 * dividers do the same thing), prefer changing the M divider over the P
 * divider (since changing both the PD and P dividers at the same time can
 * cause instability), and prefer changing all dividers over changing the
 * parent.
 *
 * @param clock   The clock description.
 * @param factors A pointer to the structure where factors are written.
 * @param rate    The requested clock rate.
 * @return        The calculated clock rate, or zero if no rate was found.
 */
static uint32_t
find_factors(struct sunxi_ccu_clock *clock, struct sunxi_ccu_factors *factors,
             uint32_t rate)
{
	uint8_t  mux_max    = BIT(BF_WIDTH(clock->mux));
	uint8_t  m_max      = BIT(BF_WIDTH(clock->m));
	uint8_t  p_max      = BIT(BF_WIDTH(clock->p));
	int32_t  best_error = INT32_MAX;
	uint32_t best_rate  = 0;
	uint32_t min_rate   = clock->info.min_rate;
	uint32_t max_rate   = clock->info.max_rate;

	/* Iterate through each combination of parent and factors twice. */
	for (size_t i = 0; i < mux_max; ++i) {
		const struct clock_handle *parent = &clock->parents[i];
		uint8_t  pd_max;
		uint32_t parent_rate;

		/* Skip mux index where there is no parent. */
		if (parent->dev == NULL)
			continue;
		/* Start calculations based on the parent clock's rate. */
		if (clock_get_rate(parent->dev, parent->id, &parent_rate))
			continue;
		pd_max = BIT(BF_WIDTH(parent->vdiv));
		/* Calculate factors to get closest to the requested rate. */
		for (uint8_t p = 0; p < p_max; ++p) {
			uint32_t p_rate = parent_rate >> p;
			for (uint8_t m = 0; m < m_max; ++m) {
				uint32_t m_rate = p_rate / (m + 1);
				for (uint8_t pd = 0; pd < pd_max; ++pd) {
					int32_t  error;
					uint32_t new_rate = m_rate / (pd + 1);
					/* Skip anything out of range. */
					if (min_rate && new_rate < min_rate)
						continue;
					if (max_rate && new_rate > max_rate)
						continue;
					/* Get the absolute value of error. */
					if ((error = rate - new_rate) < 0)
						error = -error;
					/* Skip factors with worse error. */
					if (error >= best_error)
						continue;
					best_error   = error;
					best_rate    = new_rate;
					factors->mux = i;
					factors->pd  = pd;
					factors->m   = m;
					factors->p   = p;
					/* Stop if an exact match is found. */
					if (new_rate == rate)
						return new_rate;
				}
			}
		}
	}

	return best_rate;
}

static struct clock_info *
sunxi_ccu_get_info(struct device *dev, uint8_t id)
{
	return &get_clock(dev, id)->info;
}

static struct clock_handle *
sunxi_ccu_get_parent(struct device *dev, uint8_t id)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	size_t index = 0;

	if (BF_PRESENT(clock->mux)) {
		uint32_t reg = mmio_read32(dev->regs + clock->reg);
		index = bitfield_get(reg, clock->mux);
	}

	return &clock->parents[index];
}

static int
sunxi_ccu_get_rate(struct device *dev, uint8_t id, uint32_t *rate)
{
	struct clock_handle    *parent = sunxi_ccu_get_parent(dev, id);
	struct sunxi_ccu_clock *clock  = get_clock(dev, id);
	int err;
	uint32_t reg, tmp;

	/* If a clock has no parent, it runs at a fixed rate. Return that. */
	if (parent == NULL) {
		*rate = clock->info.max_rate;
		return SUCCESS;
	}

	/* Otherwise, the rate is the parent's rate divided by some factors. */
	if ((err = clock_get_rate(parent->dev, parent->id, &tmp)))
		return err;
	reg   = mmio_read32(dev->regs + clock->reg);
	tmp  /= bitfield_get(reg, parent->vdiv) + 1;
	tmp  /= bitfield_get(reg, clock->m) + 1;
	tmp >>= bitfield_get(reg, clock->p);
	*rate = tmp;

	return SUCCESS;
}

static int
sunxi_ccu_get_state(struct device *dev, uint8_t id)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Check the bus clock gate. */
	if (gate != 0 && !bitmap_get(dev->regs, gate))
		return false;
	/* Check the reset line. */
	if (reset != 0 && !bitmap_get(dev->regs, reset))
		return false;

	return true;
}

static int
sunxi_ccu_set_rate(struct device *dev, uint8_t id, uint32_t rate)
{
	struct sunxi_ccu_clock  *clock = get_clock(dev, id);
	struct sunxi_ccu_factors factors;
	int err;
	uint32_t chosen_rate, old_rate, old_reg, reg;

	/* Find the best configuration for the clock at the desired rate. */
	chosen_rate = find_factors(clock, &factors, rate);
	/* Ensure some valid rate was possible with this clock's factors. */
	if (chosen_rate == 0)
		return ERANGE;
	debug("%s: set_rate(%s, %u) chose mux=%u pd=%u m=%u p=%u → %u",
	      dev->name, clock->info.name, rate, factors.mux,
	      factors.pd, factors.m, factors.p, chosen_rate);
	/* If the chosen rate is the same as the existing rate, do nothing. */
	if ((err = sunxi_ccu_get_rate(dev, id, &old_rate)))
		return err;
	if (chosen_rate == old_rate)
		return SUCCESS;

	/* Set the dividers for this clock. */
	reg = old_reg = mmio_read32(dev->regs + clock->reg);
	reg = bitfield_set(reg, clock->parents[factors.mux].vdiv, factors.pd);
	reg = bitfield_set(reg, clock->m, factors.m);
	reg = bitfield_set(reg, clock->p, factors.p);
	if (reg != old_reg) {
		mmio_write32(dev->regs + clock->reg, reg);
		udelay(1);
		if (mmio_read32(dev->regs + clock->reg) != reg)
			return EIO;
	}

	/* Set the parent in the mux for this clock. */
	reg = bitfield_set(reg, clock->mux, factors.mux);
	if (reg != old_reg) {
		mmio_write32(dev->regs + clock->reg, reg);
		udelay(1);
		if (mmio_read32(dev->regs + clock->reg) != reg)
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_set_state(struct device *dev, uint8_t id, bool enable)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	if (enable) {
		/* Enable the clock before taking the device out of reset. */
		if (gate != 0) {
			bitmap_set(dev->regs, gate);
			if (!bitmap_get(dev->regs, gate))
				return EIO;
		}
		/* Deassert the reset once the device has a running clock. */
		if (reset != 0) {
			bitmap_set(dev->regs, reset);
			if (!bitmap_get(dev->regs, reset))
				return EIO;
		}
	} else {
		/* Put the device in reset before turning off its clock. */
		if (reset != 0) {
			bitmap_clear(dev->regs, reset);
			if (bitmap_get(dev->regs, reset))
				return EIO;
		}
		/* Finally gate the bus clock. */
		if (gate != 0) {
			bitmap_clear(dev->regs, gate);
			if (bitmap_get(dev->regs, gate))
				return EIO;
		}
	}

	return SUCCESS;
}

static int
sunxi_ccu_probe(struct device *dev __unused)
{
	/* Ensure a list of clock descriptions was provided. */
	assert(dev->drvdata);

	return SUCCESS;
}

const struct clock_driver sunxi_ccu_driver = {
	.drv = {
		.class = DM_CLASS_CLOCK,
		.probe = sunxi_ccu_probe,
	},
	.ops = {
		.get_info   = sunxi_ccu_get_info,
		.get_parent = sunxi_ccu_get_parent,
		.get_rate   = sunxi_ccu_get_rate,
		.get_state  = sunxi_ccu_get_state,
		.set_rate   = sunxi_ccu_set_rate,
		.set_state  = sunxi_ccu_set_state,
	},
};
