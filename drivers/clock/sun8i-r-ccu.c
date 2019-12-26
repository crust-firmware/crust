/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <debug.h>
#include <device.h>
#include <error.h>
#include <mmio.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

static const uint32_t sun8i_r_ccu_fixed_rates[] = {
	[CLK_OSC16M] = 16000000U,
	[CLK_OSC24M] = 24000000U,
	[CLK_OSC32K] = 32768U,
};

static uint32_t
sun8i_r_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                           uint32_t rate UNUSED, uint8_t id)
{
	assert(id < ARRAY_SIZE(sun8i_r_ccu_fixed_rates));

	return sun8i_r_ccu_fixed_rates[id];
}

static uint32_t
sun8i_r_ccu_ar100_get_rate(const struct ccu *self, uint32_t rate, uint8_t id)
{
	const struct ccu_clock *clk = &self->clocks[id];
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	/* Parent 2 (CLK_PLL_PERIPH0) has an additional divider. */
	if (bitfield_get(val, clk->mux) == 2)
		rate /= bitfield_get(val, BITFIELD(8, 5)) + 1;

	return rate;
}

static const struct ccu_clock sun8i_r_ccu_clocks[SUN8I_R_CCU_CLOCKS] = {
	[CLK_OSC16M] = {
		.get_rate = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC24M] = {
		.get_rate = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC32K] = {
		.get_rate = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_AR100] = {
		.parents = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = CLK_OSC32K },
			{ .dev = &r_ccu.dev, .id = CLK_OSC24M },
			{ .dev = &ccu.dev, .id = CLK_PLL_PERIPH0 },
			{ .dev = &r_ccu.dev, .id = CLK_OSC16M },
		},
		.get_rate = sun8i_r_ccu_ar100_get_rate,
		.reg      = 0x0000,
		.mux      = BITFIELD(16, 2),
		.p        = BITFIELD(4, 2),
	},
	[CLK_AHB0] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_AR100),
	},
	[CLK_APB0] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_AHB0),
		.reg     = 0x000c,
		.p       = BITFIELD(0, 2),
	},
	[CLK_BUS_R_PIO] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 0),
	},
	[CLK_BUS_R_CIR] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 1),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 1),
	},
	[CLK_BUS_R_TIMER] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 2),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 2),
	},
	[CLK_BUS_R_RSB] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 3),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 3),
	},
	[CLK_BUS_R_UART] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 4),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 4),
	},
	[CLK_BUS_R_I2C] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 6),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 6),
	},
	[CLK_BUS_R_TWD] = {
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 7),
	},
	[CLK_R_CIR] = {
		.parents = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = CLK_OSC32K },
			{ .dev = &r_ccu.dev, .id = CLK_OSC24M },
		},
		.gate = BITMAP_INDEX(0x0054 >> 2, 31),
		.reg  = 0x0054,
		.mux  = BITFIELD(24, 2),
		.m    = BITFIELD(0, 4),
		.p    = BITFIELD(16, 2),
	},
};

const struct ccu r_ccu = {
	.dev = {
		.name  = "r_ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_R_CCU_CLOCKS),
	},
	.clocks = sun8i_r_ccu_clocks,
	.regs   = DEV_R_PRCM,
};
