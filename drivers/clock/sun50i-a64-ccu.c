/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <error.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

#define CPUX_AXI_CFG_REG  0x0050
#define AHB1_APB1_CFG_REG 0x0054
#define APB2_CFG_REG      0x0058
#define AHB2_CFG_REG      0x005c

#define CPUX_CLK_SRC(x)   ((x) << 16)
#define CPUX_APB_CLK_M(x) ((x) << 8)
#define AXI_CLK_M(x)      ((x) << 0)

#define AHB1_CLK_SRC(x)   ((x) << 12)
#define APB1_CLK_DIV(x)   ((x) << 8)
#define AHB1_PRE_DIV(x)   ((x) << 6)
#define AHB1_CLK_P(x)     ((x) << 4)

#define APB2_CLK_SRC(x)   ((x) << 24)
#define APB2_CLK_P(x)     ((x) << 16)
#define APB2_CLK_M(x)     ((x) << 0)

#define AHB2_CLK_SRC(n)   ((n) << 0)

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
#if CONFIG(SOC_A64)
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
#if CONFIG(SOC_A64)
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

static int
sun50i_a64_ccu_probe(const struct device *dev)
{
	const struct ccu *self = to_ccu(dev);

	/* Set CPUX to PLL_CPUX, APB to CPUX/4, AXI to CPUX/3. */
	mmio_write_32(self->regs + CPUX_AXI_CFG_REG,
	              CPUX_CLK_SRC(2) |
	              CPUX_APB_CLK_M(3) |
	              AXI_CLK_M(2));

	/* Set AHB1 to PLL_PERIPH0/3 (200MHz), APB1 to AHB1/2 (100MHz). */
	mmio_write_32(self->regs + AHB1_APB1_CFG_REG,
	              AHB1_CLK_SRC(3) |
	              APB1_CLK_DIV(1) |
	              AHB1_PRE_DIV(2) |
	              AHB1_CLK_P(0));

	/* Set APB2 to OSC24M/1 (24MHz). */
	mmio_write_32(self->regs + APB2_CFG_REG,
	              APB2_CLK_SRC(1) |
	              APB2_CLK_P(0) |
	              APB2_CLK_M(0));

	/* Set AHB2 to PLL_PERIPH0/2 (300MHz). */
	mmio_write_32(self->regs + AHB2_CFG_REG,
	              AHB2_CLK_SRC(1));

	return SUCCESS;
}

static void
sun50i_a64_ccu_release(const struct device *dev)
{
	const struct ccu *self = to_ccu(dev);

	/* Set AHB1 to LOSC/1 (32kHz), APB1 to AHB1/2 (16kHz). */
	mmio_write_32(self->regs + AHB1_APB1_CFG_REG,
	              AHB1_CLK_SRC(0) |
	              APB1_CLK_DIV(1) |
	              AHB1_PRE_DIV(2) |
	              AHB1_CLK_P(0));

	/* Set AHB2 to AHB1/1 (32kHz). */
	mmio_write_32(self->regs + AHB2_CFG_REG,
	              AHB2_CLK_SRC(0));
}

static const struct clock_driver sun50i_a64_ccu_driver = {
	.drv = {
		.probe   = sun50i_a64_ccu_probe,
		.release = sun50i_a64_ccu_release,
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
