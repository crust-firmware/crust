/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
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

#define IS_LAST_CLOCK(clock) ((clock)->flags & SUNXI_CCU_FLAG_LAST)

struct sunxi_ccu_factors {
	size_t   parent_index; /**< Mux index for the parent. */
	uint32_t parent_rate;  /**< The expected rate of the parent clock. */
	uint8_t  pd;           /**< Linear parent PD ("pre-divider"). */
	uint8_t  m;            /**< Linear M ("multiple") divider. */
	uint8_t  p;            /**< Exponential P ("power") divider. */
};

static uint32_t find_factors(struct device *dev, struct sunxi_ccu_clock *clock,
                             struct sunxi_ccu_factors *factors, uint32_t rate);
static struct sunxi_ccu_clock *get_clock(struct device *dev, uint8_t id);
static uint32_t get_max_rate(struct device *dev,
                             struct sunxi_ccu_clock *clock);
static size_t get_parent_index(struct device *dev,
                               struct sunxi_ccu_clock *clock);
static int sunxi_ccu_disable_id(struct device *dev, int id);
static int sunxi_ccu_enable_id(struct device *dev, int id, uint32_t rate);
static uint32_t sunxi_ccu_get_rate_id(struct device *dev, uint8_t id);
static int sunxi_ccu_set_rate_id(struct device *dev, uint8_t id,
                                 uint32_t rate);

/*
 * Choose the parent and set of dividers that will configure the clock to run
 * as close as possible to the desired rate.
 *
 * Iterate through each possible parent and each combination of dividers
 * (including a pre-divider if present for that parent) until an exact match
 * is found. If no exact match is found, return the best match that is below
 * this clock's maximum rate.
 *
 * Order the iterations to favor changing PD divider over the M divider (since
 * use of the M divider is sometimes recommended against, and the PD and M
 * dividers do the same thing), prefer changing the M divider over the P
 * divider (since changing both the PD and P dividers at the same time can
 * cause instability), and prefer changing all dividers over changing the
 * parent.
 *
 * Since clocks do not have multiplier factors, only parents with a maximum
 * rate at least as high as the requested reate are considered.
 *
 * @param dev     The CCU device.
 * @param clock   The clock description.
 * @param factors A pointer to the structure where factors are written.
 * @param rate    The requested clock rate.
 */
static uint32_t
find_factors(struct device *dev, struct sunxi_ccu_clock *clock,
             struct sunxi_ccu_factors *factors, uint32_t rate)
{
	int32_t  best_error = INT32_MAX;
	size_t   best_parent_index;
	size_t   orig_parent_index = get_parent_index(dev, clock);
	uint8_t  m_max     = BIT(BF_WIDTH(clock->m));
	uint8_t  p_max     = BIT(BF_WIDTH(clock->p));
	uint32_t best_rate = 0;
	uint32_t max_rate  = get_max_rate(dev, clock);

	/* Iterate through each combination of parent and factors twice. */
	for (size_t i = 0; i < 2 * SUNXI_CCU_PARENT_MAX; ++i) {
		size_t   parent_index;
		uint8_t  parent_id, pd_max;
		uint32_t parent_max;

		/* Start at the current parent. */
		parent_index = (orig_parent_index + i) % SUNXI_CCU_PARENT_MAX;
		parent_id    = clock->parents[parent_index];
		/* Skip mux index where there is no parent. */
		if (parent_id == SUNXI_CCU_NONE)
			continue;
		/* Use parent current rate the first round, then max rate. */
		if (i < SUNXI_CCU_PARENT_MAX) {
			parent_max = sunxi_ccu_get_rate_id(dev, parent_id);
		} else {
			static struct sunxi_ccu_clock *parent_clock;
			parent_clock = get_clock(dev, parent_id);
			parent_max   = get_max_rate(dev, parent_clock);
		}
		pd_max = BIT(BF_WIDTH(clock->pd[parent_index]));
		/* Calculate factors to get closest to the requested rate. */
		for (uint8_t p = 0; p < p_max; ++p) {
			uint32_t rate1 = parent_max >> p;
			for (uint8_t m = 0; m < m_max; ++m) {
				uint32_t rate2 = rate1 / (m + 1);
				for (uint8_t pd = 0; pd < pd_max; ++pd) {
					int32_t  error;
					uint32_t new_rate = rate2 / (pd + 1);
					/* Skip anything above the max rate. */
					if (new_rate > max_rate)
						continue;
					if ((error = rate - new_rate) < 0)
						error = -error;
					/* Skip choices with worse error. */
					if (error >= best_error)
						continue;
					best_error           = error;
					best_parent_index    = parent_index;
					best_rate            = new_rate;
					factors->parent_rate = parent_max;
					factors->pd          = pd;
					factors->m           = m;
					factors->p           = p;
					/* Stop if an exact match is found. */
					if (new_rate == rate)
						goto exact;
				}
			}
		}
	}
	if (best_rate > 0 && best_error >= (int32_t)(rate / 20))
		warn("%s: Selected rate %u is >5%% from %u", dev->name,
		     best_rate, rate);

exact:
	/* Fill in the reset of the factors struct. */
	factors->parent_index = best_parent_index;

	return best_rate;
}

/**
 * Get the structure reprensenting a clock from the CCU device and the clock's
 * numerical ID.
 *
 * @param dev The CCU device.
 * @param id  The numerical clock ID—must not be SUNXI_CCU_NONE.
 */
static struct sunxi_ccu_clock *
get_clock(struct device *dev, uint8_t id)
{
	assert(id < SUNXI_CCU_NONE);

	return &((struct sunxi_ccu_clock *)(dev->drvdata))[id];
}

/**
 * Get the maximum possible rate of this clock. If no maximum rate is
 * defined in driver data, this is the rate available when the fastest parent
 * is also running at its maximum rate, and all dividers are at unity.
 *
 * @param dev   The CCU device.
 * @param clock The clock description.
 */
static uint32_t
get_max_rate(struct device *dev, struct sunxi_ccu_clock *clock)
{
	uint32_t max_rate = 0, parent_max;

	/* Use the cached or pre-assigned maximum rate if there is one. */
	if (clock->max_rate > 0)
		return clock->max_rate;

	/* Choose the highest maximum rate of all parents. */
	for (size_t i = 0; i < SUNXI_CCU_PARENT_MAX; ++i) {
		uint8_t parent_id = clock->parents[i];
		if (clock->parents[i] == SUNXI_CCU_NONE)
			continue;
		parent_max = get_max_rate(dev, get_clock(dev, parent_id));
		if (parent_max > max_rate)
			max_rate = parent_max;
	}
	/* If there's no rate control, the maximum rate is the current rate. */
	if (max_rate == 0)
		max_rate = clock->rate;

	/* Record the calculated maximum rate in the clock struct. */
	clock->max_rate = max_rate;

	return max_rate;
}

/**
 * Get the index in the mux of the current parent clock. If this clock has no
 * mux, this function returns 0.
 *
 * @param dev   The CCU device.
 * @param clock The clock description.
 */
static size_t
get_parent_index(struct device *dev, struct sunxi_ccu_clock *clock)
{
	if (BF_PRESENT(clock->mux)) {
		uint32_t reg = mmio_read32(dev->regs + clock->reg);
		return bitfield_get(reg, clock->mux);
	}

	return 0;
}

static int
sunxi_ccu_disable_id(struct device *dev, int id)
{
	struct sunxi_ccu_clock *child;
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Not allowed to modify fixed clocks. */
	if (clock->flags & SUNXI_CCU_FLAG_FIXED)
		return EPERM;
	/* Do nothing when the clock is already disabled. */
	if (!(clock->flags & SUNXI_CCU_FLAG_ACTIVE))
		return SUCCESS;
	/* Prevent disabling the clock if any children are in use. */
	for (size_t i = 0; !IS_LAST_CLOCK(child = get_clock(dev, i)); ++i) {
		if (child->parents[get_parent_index(dev, child)] != id)
			continue;
		if (child->flags & SUNXI_CCU_FLAG_ACTIVE)
			return EBUSY;
	}

	/* Put the device in reset before turning off its clock. */
	if (reset != 0) {
		bitmap_clear(dev->regs, reset);
		if (bitmap_get(dev->regs, reset))
			return EIO;
	}
	/* Disable any special module clock gate before the bus clock. */
	if (clock->flags & SUNXI_CCU_FLAG_GATED) {
		mmio_clearbits32(dev->regs + clock->reg, BIT(31));
		if (mmio_read32(dev->regs + clock->reg) & BIT(31))
			return EIO;
	}
	/* Finally disable the bus clock gate. */
	if (gate != 0) {
		bitmap_clear(dev->regs, gate);
		if (bitmap_get(dev->regs, gate))
			return EIO;
	}

	/* Mark the clock as no longer running. */
	clock->flags &= ~SUNXI_CCU_FLAG_ACTIVE;
	clock->rate   = 0;

	return SUCCESS;
}

static int
sunxi_ccu_enable_id(struct device *dev, int id, uint32_t rate)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Do nothing when no rate given and the clock is already active. */
	if (clock->flags & SUNXI_CCU_FLAG_ACTIVE)
		return SUCCESS;
	/* Not allowed to modify fixed clocks. */
	if (clock->flags & SUNXI_CCU_FLAG_FIXED)
		return EPERM;

	/* Set the clock to its requested rate. This takes care of choosing a
	 * parent and ensuring that the parent is enabled. Setting the rate
	 * will fail for fixed clocks and those without rate control, but those
	 * must be set up in advance anyway. Only propagate the error forward
	 * if it signifies a hardware issue. */
	if (sunxi_ccu_set_rate_id(dev, id, rate) == EIO)
		return EIO;
	/* Enable the clock before taking the device out of reset. */
	if (gate != 0) {
		bitmap_set(dev->regs, gate);
		if (!bitmap_get(dev->regs, gate))
			return EIO;
	}
	/* Enable any special module clock gate after the bus clock. */
	if (clock->flags & SUNXI_CCU_FLAG_GATED) {
		mmio_setbits32(dev->regs + clock->reg, BIT(31));
		if (!(mmio_read32(dev->regs + clock->reg) & BIT(31)))
			return EIO;
	}
	/* Only deassert the reset once the device has a running clock. */
	if (reset != 0) {
		bitmap_set(dev->regs, reset);
		if (!bitmap_get(dev->regs, reset))
			return EIO;
	}

	/* Mark the clock as being in use. */
	clock->flags |= SUNXI_CCU_FLAG_ACTIVE;

	return SUCCESS;
}

static uint32_t
sunxi_ccu_get_rate_id(struct device *dev, uint8_t id)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	size_t   parent_index;
	uint32_t rate, reg;

	/* Use the cached or pre-assigned fixed rate if there is one. */
	if (clock->rate > 0)
		return clock->rate;

	parent_index = get_parent_index(dev, clock);
	/* If the clock has no parent or fixed rate, it must not be running. */
	if (clock->parents[parent_index] == SUNXI_CCU_NONE)
		return 0;
	/* Calculate the current clock rate from its parent and dividers. */
	rate   = sunxi_ccu_get_rate_id(dev, clock->parents[parent_index]);
	reg    = mmio_read32(dev->regs + clock->reg);
	rate  /= bitfield_get(reg, clock->pd[parent_index]) + 1;
	rate  /= bitfield_get(reg, clock->m) + 1;
	rate >>= bitfield_get(reg, clock->p);

	/* Cache the current rate for future queries. */
	clock->rate = rate;

	return rate;
}

static int
sunxi_ccu_set_rate_id(struct device *dev, uint8_t id, uint32_t rate)
{
	struct sunxi_ccu_clock  *clock = get_clock(dev, id);
	struct sunxi_ccu_factors factors;
	uint8_t  parent_id;
	uint32_t new_rate, old_rate, orig_reg, reg;

	/* Not allowed to modify fixed clocks. */
	if (clock->flags & SUNXI_CCU_FLAG_FIXED)
		return EPERM;

	old_rate = sunxi_ccu_get_rate_id(dev, id);
	if (rate == 0) {
		uint32_t max_rate = get_max_rate(dev, clock);
		/* Try to keep the current rate unless it's out of range. */
		rate = old_rate <= max_rate ? old_rate : max_rate;
	}
	/* Find the best configuration for the clock at this rate. */
	new_rate = find_factors(dev, clock, &factors, rate);
	/* Ensure some valid rate was possible with this clock's factors. */
	if (new_rate == 0)
		return ERANGE;
	debug("%s: set_rate(%u, %u) chose mux=%u PD=%u M=%u P=%u -> %u",
	      dev->name, id, rate, factors.parent_index, factors.pd, factors.m,
	      factors.p, new_rate);
	/* Now that the parent is known, enable it and raise its rate. */
	parent_id = clock->parents[factors.parent_index];
	if (sunxi_ccu_enable_id(dev, parent_id, factors.parent_rate) == EIO)
		return EIO;
	if (new_rate != old_rate) {
		struct sunxi_ccu_clock *child;
		size_t i;

		/* Update the cached rate with the new calculated rate. */
		clock->rate = new_rate;
		/* Update the dividers of active children for the new rate. */
		for (i = 0; !IS_LAST_CLOCK(child = get_clock(dev, i)); ++i) {
			if (child->parents[get_parent_index(dev, child)] != id)
				continue;
			if (!(child->flags & SUNXI_CCU_FLAG_ACTIVE))
				continue;
			/* The child's cached rate is now wrong; clear it. */
			child->rate = 0;
			/* Ensure the child is running below its max rate. */
			if (sunxi_ccu_set_rate_id(dev, i, 0) == EIO)
				warn("%s: set_rate(%u) update child %u fail!",
				     dev->name, id, i);
		}
	}

	/* Set the dividers for this clock. */
	reg = orig_reg = mmio_read32(dev->regs + clock->reg);
	reg = bitfield_set(reg, clock->pd[factors.parent_index], factors.pd);
	reg = bitfield_set(reg, clock->m, factors.m);
	reg = bitfield_set(reg, clock->p, factors.p);
	if (reg != orig_reg) {
		mmio_write32(dev->regs + clock->reg, reg);
		udelay(1);
		if (mmio_read32(dev->regs + clock->reg) != reg)
			return EIO;
	}
	/* If this clock has a mux, update that. */
	if (BF_PRESENT(clock->mux)) {
		reg = bitfield_set(reg, clock->mux, factors.parent_index);
		if (reg != orig_reg) {
			mmio_write32(dev->regs + clock->reg, reg);
			udelay(1);
			if (mmio_read32(dev->regs + clock->reg) != reg)
				return EIO;
		}
	}

	return SUCCESS;
}

static int
sunxi_ccu_disable(struct device *dev, uint8_t id)
{
	return sunxi_ccu_disable_id(dev, id);
}

static int
sunxi_ccu_enable(struct device *dev, uint8_t id)
{
	return sunxi_ccu_enable_id(dev, id, 0);
}

static int
sunxi_ccu_get_rate(struct device *dev, uint8_t id, uint32_t *rate)
{
	*rate = sunxi_ccu_get_rate_id(dev, id);

	return SUCCESS;
}

static int
sunxi_ccu_set_rate(struct device *dev, uint8_t id, uint32_t rate)
{
	return sunxi_ccu_set_rate_id(dev, id, rate);
}

static int
sunxi_ccu_probe(struct device *dev __unused)
{
	assert(dev->drvdata);

	return SUCCESS;
}

const struct clock_driver sunxi_ccu_driver = {
	.drv = {
		.name  = "sunxi-ccu",
		.class = DM_CLASS_CLOCK,
		.probe = sunxi_ccu_probe,
	},
	.ops = {
		.disable  = sunxi_ccu_disable,
		.enable   = sunxi_ccu_enable,
		.get_rate = sunxi_ccu_get_rate,
		.set_rate = sunxi_ccu_set_rate,
	},
};
