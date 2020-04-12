/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

static uint32_t
sun50i_a64_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                              const struct ccu_clock *clk UNUSED,
                              uint32_t rate UNUSED)
{
	return 600000000U;
}

static const struct clock_handle sun50i_a64_ccu_cpux_parents[] = {
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC32K,
	},
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC24M,
	},
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_CPUX,
	},
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_CPUX,
	},
};

static const struct clock_handle *
sun50i_a64_ccu_cpux_get_parent(const struct ccu *self,
                               const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun50i_a64_ccu_cpux_parents[bitfield_get(val, 16, 2)];
}

static const struct clock_handle sun50i_a64_ccu_dram_parents[] = {
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_DDR0,
	},
#if CONFIG_SOC_A64
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_DDR1,
	},
#else
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_PERIPH0,
	},
#endif
};

static const struct clock_handle *
sun50i_a64_ccu_dram_get_parent(const struct ccu *self,
                               const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun50i_a64_ccu_dram_parents[bitfield_get(val, 20, 1)];
}

static const struct ccu_clock sun50i_a64_ccu_clocks[SUN50I_A64_CCU_CLOCKS] = {
	[CLK_PLL_CPUX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.reg        = 0x0000,
		.lock       = 28,
		.gate       = BITMAP_INDEX(0x0000, 31),
	},
	[CLK_PLL_DDR0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.reg        = 0x0020,
		.lock       = 28,
		.update     = 20,
		.gate       = BITMAP_INDEX(0x0020, 31),
	},
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_a64_ccu_fixed_get_rate,
	},
#if CONFIG_SOC_A64
	[CLK_PLL_DDR1] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.reg        = 0x004c,
		.lock       = 28,
		.update     = 30,
		.gate       = BITMAP_INDEX(0x004c, 31),
	},
#endif
	[CLK_CPUX] = {
		.get_parent = sun50i_a64_ccu_cpux_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.reg        = 0x0050,
	},
	[CLK_BUS_DRAM] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0060, 14),
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0064, 21),
		.reset      = BITMAP_INDEX(0x02c4, 21),
	},
	[CLK_DRAM] = {
		.get_parent = sun50i_a64_ccu_dram_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.reg        = 0x00f4,
		.update     = 16,
		.reset      = BITMAP_INDEX(0x00f4, 31),
	},
	[CLK_MBUS] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x015c, 31),
		.reset      = BITMAP_INDEX(0x00fc, 31),
	},
};

static const struct clock_driver sun50i_a64_ccu_driver = {
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

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &sun50i_a64_ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_A64_CCU_CLOCKS),
	},
	.clocks = sun50i_a64_ccu_clocks,
	.regs   = DEV_CCU,
};
