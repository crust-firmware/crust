/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
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
#define CPUX_AXI_CLK_M(x) ((x) << 0)

#define AHB1_CLK_SRC(x)   ((x) << 12)
#define APB1_CLK_DIV(x)   ((x) << 8)
#define AHB1_PRE_DIV(x)   ((x) << 6)
#define AHB1_CLK_P(x)     ((x) << 4)

#define APB2_CLK_SRC(x)   ((x) << 24)
#define APB2_CLK_P(x)     ((x) << 16)
#define APB2_CLK_M(x)     ((x) << 0)

#define AHB2_CLK_SRC(n)   ((n) << 0)

static DEFINE_FIXED_RATE(ccu_get_pll_periph0_rate, 600000000U)

/*
 * While APB2 has a mux, assume its parent is OSC24M. Reparenting APB2
 * to PLL_PERIPH0 in Linux for faster UART clocks is unsupported.
 */
static DEFINE_FIXED_PARENT(ccu_get_apb2_parent, r_ccu, CLK_OSC24M)
static DEFINE_FIXED_PARENT(ccu_get_apb2, ccu, CLK_APB2)

static const struct clock_handle ccu_dram_parents[] = {
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_DDR0,
	},
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_PERIPH0, /* 2x */
	},
};

static const struct clock_handle *
ccu_get_dram_parent(const struct ccu *self,
                    const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &ccu_dram_parents[bitfield_get(val, 20, 1)];
}

static const struct ccu_clock ccu_clocks[SUN8I_H3_CCU_CLOCKS] = {
	[CLK_PLL_CPUX] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.reg        = 0x0000,
		.lock       = 28,
		.gate       = BITMAP_INDEX(0x0000, 31),
	},
	[CLK_PLL_DDR0] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.reg        = 0x0020,
		.lock       = 28,
		.update     = 20,
		.gate       = BITMAP_INDEX(0x0020, 31),
	},
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_pll_periph0_rate,
	},
	[CLK_APB2] = {
		.get_parent = ccu_get_apb2_parent,
		.get_rate   = ccu_get_parent_rate,
	},
	/* Reset requires re-training DRAM, so ignore it. */
	[CLK_BUS_DRAM] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x0060, 14),
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x0064, 21),
		.reset      = BITMAP_INDEX(0x02c4, 21),
	},
	[CLK_BUS_PIO] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x0068, 5),
	},
#if CONFIG(SERIAL_DEV_UART0)
	[CLK_BUS_UART0] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x006c, 16),
		.reset      = BITMAP_INDEX(0x02d8, 16),
	},
#elif CONFIG(SERIAL_DEV_UART1)
	[CLK_BUS_UART1] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x006c, 17),
		.reset      = BITMAP_INDEX(0x02d8, 17),
	},
#elif CONFIG(SERIAL_DEV_UART2)
	[CLK_BUS_UART2] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x006c, 18),
		.reset      = BITMAP_INDEX(0x02d8, 18),
	},
#elif CONFIG(SERIAL_DEV_UART3)
	[CLK_BUS_UART3] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x006c, 19),
		.reset      = BITMAP_INDEX(0x02d8, 19),
	},
#endif
	[CLK_DRAM] = {
		.get_parent = ccu_get_dram_parent,
		.get_rate   = ccu_get_parent_rate,
		.reg        = 0x00f4,
		.update     = 16,
		.reset      = BITMAP_INDEX(0x00f4, 31),
	},
	/* MBUS reset breaks DRAM resume on H3. */
	[CLK_MBUS] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x015c, 31),
	},
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_H3_CCU_CLOCKS),
	},
	.clocks = ccu_clocks,
	.regs   = DEV_CCU,
};

static const struct clock_handle pll_cpux = {
	.dev = &ccu.dev,
	.id  = CLK_PLL_CPUX,
};

void
ccu_suspend(void)
{
	/* Set AHB1 to LOSC/1 (32kHz), APB1 to AHB1/2 (16kHz). */
	mmio_write_32(DEV_CCU + AHB1_APB1_CFG_REG,
	              AHB1_CLK_SRC(0) |
	              APB1_CLK_DIV(1) |
	              AHB1_PRE_DIV(2) |
	              AHB1_CLK_P(0));

	/* Set AHB2 to AHB1/1 (32kHz). */
	mmio_write_32(DEV_CCU + AHB2_CFG_REG,
	              AHB2_CLK_SRC(0));

	clock_put(&pll_cpux);
}

void
ccu_suspend_cluster(uint32_t cluster UNUSED)
{
	/* Set CPUX to LOSC (32kHz), APB to CPUX/4, AXI to CPUX/3. */
	mmio_write_32(DEV_CCU + CPUX_AXI_CFG_REG,
	              CPUX_CLK_SRC(0) |
	              CPUX_APB_CLK_M(3) |
	              CPUX_AXI_CLK_M(2));
}

void
ccu_resume(void)
{
	clock_get(&pll_cpux);

	/* Set AHB1 to PLL_PERIPH0/3 (200MHz), APB1 to AHB1/2 (100MHz). */
	mmio_write_32(DEV_CCU + AHB1_APB1_CFG_REG,
	              AHB1_CLK_SRC(3) |
	              APB1_CLK_DIV(1) |
	              AHB1_PRE_DIV(2) |
	              AHB1_CLK_P(0));

	/* Set AHB2 to PLL_PERIPH0/2 (300MHz). */
	mmio_write_32(DEV_CCU + AHB2_CFG_REG,
	              AHB2_CLK_SRC(1));
}

void
ccu_resume_cluster(uint32_t cluster UNUSED)
{
	/* Set CPUX to PLL_CPUX, APB to CPUX/4, AXI to CPUX/3. */
	mmio_write_32(DEV_CCU + CPUX_AXI_CFG_REG,
	              CPUX_CLK_SRC(2) |
	              CPUX_APB_CLK_M(3) |
	              CPUX_AXI_CLK_M(2));
}

void
ccu_init(void)
{
	/* Set APB2 to OSC24M/1 (24MHz). */
	mmio_write_32(DEV_CCU + APB2_CFG_REG,
	              APB2_CLK_SRC(1) |
	              APB2_CLK_P(0) |
	              APB2_CLK_M(0));

	ccu_resume();
	ccu_resume_cluster(0);
}
