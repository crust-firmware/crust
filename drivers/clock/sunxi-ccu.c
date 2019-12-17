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
