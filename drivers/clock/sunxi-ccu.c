/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <debug.h>
#include <delay.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <clock/sunxi-ccu.h>
#include <platform/devices.h>

struct sunxi_ccu_clock {
	/** Generic clock information shared by all drivers. */
	struct clock_info          info;
	/** Handles to parent clocks (one for each possible mux value). */
	const struct clock_handle *parents;
	/** Offset into the CCU of the clock gate bit, zero for none. */
	const uint16_t             gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	const uint16_t             reset;
	/** Offset into the CCU of the mux/factor register. */
	const uint16_t             reg;
	/** Offset and width of the parent mux control in the register. */
	const bitfield_t           mux;
	/** Offset and width of the linear divider in the register. */
	const bitfield_t           m;
	/** Offset and width of the exponential divider in the register. */
	const bitfield_t           p;
};

struct sunxi_ccu_factors {
	uint8_t mux; /**< Mux index for the parent. */
	uint8_t pd;  /**< Linear parent PD ("post-divider"). */
	uint8_t m;   /**< Linear M ("multiple") divider. */
	uint8_t p;   /**< Exponential P ("power") divider. */
};

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
sunxi_ccu_set_rate(const struct device *dev, uint8_t id, uint32_t rate)
{
	const struct sunxi_ccu  *self  = to_sunxi_ccu(dev);
	struct sunxi_ccu_clock  *clock = get_clock(self, id);
	struct sunxi_ccu_factors factors;
	uint32_t chosen_rate, old_rate, old_reg, reg;
	int err;

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
	reg = old_reg = mmio_read_32(self->regs + clock->reg);
	reg = bitfield_set(reg, clock->parents[factors.mux].vdiv, factors.pd);
	reg = bitfield_set(reg, clock->m, factors.m);
	reg = bitfield_set(reg, clock->p, factors.p);
	if (reg != old_reg) {
		mmio_write_32(self->regs + clock->reg, reg);
		udelay(1);
		if (mmio_read_32(self->regs + clock->reg) != reg)
			return EIO;
	}

	/* Set the parent in the mux for this clock. */
	reg = bitfield_set(reg, clock->mux, factors.mux);
	if (reg != old_reg) {
		mmio_write_32(self->regs + clock->reg, reg);
		udelay(1);
		if (mmio_read_32(self->regs + clock->reg) != reg)
			return EIO;
	}

	return SUCCESS;
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

static const struct clock_driver sunxi_ccu_driver = {
	.drv = {
		.probe   = dummy_probe,
		.release = dummy_release,
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

static struct sunxi_ccu_clock ccu_clocks[CCU_CLOCK_COUNT] = {
	[CCU_CLOCK_PLL_PERIPH0] = FIXED_CLOCK("pll_periph0", 600000000, 0),
	[CCU_CLOCK_MSGBOX]      = {
		.info = {
			.name  = "msgbox",
			.flags = CLK_FIXED,
		},
		.gate  = CCU_GATE_MSGBOX,
		.reset = CCU_RESET_MSGBOX,
	},
};

const struct sunxi_ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &sunxi_ccu_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clocks = ccu_clocks,
	.regs   = DEV_CCU,
};

static struct sunxi_ccu_clock r_ccu_clocks[R_CCU_CLOCK_COUNT] = {
	[R_CCU_CLOCK_OSC24M] = FIXED_CLOCK("osc24m", 24000000, 0),
	[R_CCU_CLOCK_OSC32K] = FIXED_CLOCK("osc32k", 32768, 0),
	[R_CCU_CLOCK_OSC16M] = FIXED_CLOCK("osc16m", 16000000, 0),
	[R_CCU_CLOCK_AHB0]   = {
		.info = {
			.name     = "ahb0",
			.max_rate = 300000000,
			.flags    = CLK_CRITICAL,
		},
		.parents = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = R_CCU_CLOCK_OSC32K },
			{ .dev = &r_ccu.dev, .id = R_CCU_CLOCK_OSC24M },
			{
				.dev  = &ccu.dev,
				.id   = CCU_CLOCK_PLL_PERIPH0,
				.vdiv = BITFIELD(8, 5),
			},
			{ .dev = &r_ccu.dev, .id = R_CCU_CLOCK_OSC16M },
		},
		.reg = R_CCU_CLOCK_AHB0_REG,
		.mux = BITFIELD(16, 2),
		.p   = BITFIELD(4, 2),
	},
	[R_CCU_CLOCK_APB0] = {
		.info.name = "apb0",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_AHB0),
		.reg       = R_CCU_CLOCK_APB0_REG,
		.p         = BITFIELD(0, 2),
	},
	[R_CCU_CLOCK_R_PIO] = {
		.info.name = "r_pio",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_PIO,
	},
	[R_CCU_CLOCK_R_CIR] = {
		.info = {
			.name     = "r_cir",
			.max_rate = 100000000,
		},
		.parents = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate    = R_CCU_GATE_R_CIR,
		.reset   = R_CCU_RESET_R_CIR,
	},
	[R_CCU_CLOCK_R_TIMER] = {
		.info.name = "r_timer",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_TIMER,
		.reset     = R_CCU_RESET_R_TIMER,
	},
#if CONFIG_RSB
	[R_CCU_CLOCK_R_RSB] = {
		.info.name = "r_rsb",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_RSB,
		.reset     = R_CCU_RESET_R_RSB,
	},
#endif
	[R_CCU_CLOCK_R_UART] = {
		.info.name = "r_uart",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_UART,
		.reset     = R_CCU_RESET_R_UART,
	},
	[R_CCU_CLOCK_R_I2C] = {
		.info.name = "r_i2c",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_I2C,
		.reset     = R_CCU_RESET_R_I2C,
	},
	[R_CCU_CLOCK_R_TWD] = {
		.info.name = "r_twd",
		.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
		.gate      = R_CCU_GATE_R_TWD,
	},
	[R_CCU_CLOCK_R_CIR_MOD] = {
		.info.name = "r_cir_mod",
		.parents   = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = R_CCU_CLOCK_OSC32K },
			{ .dev = &r_ccu.dev, .id = R_CCU_CLOCK_OSC24M },
		},
		.gate = BITMAP_INDEX(R_CCU_CLOCK_R_CIR_REG / 4, 31),
		.reg  = R_CCU_CLOCK_R_CIR_REG,
		.mux  = BITFIELD(24, 2),
		.m    = BITFIELD(0, 4),
		.p    = BITFIELD(16, 2),
	},
};

const struct sunxi_ccu r_ccu = {
	.dev = {
		.name  = "r_ccu",
		.drv   = &sunxi_ccu_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clocks = r_ccu_clocks,
	.regs   = DEV_R_PRCM,
};
