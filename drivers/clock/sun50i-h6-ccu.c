/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

#define CPUX_AXI_CFG_REG  0x0500
#define PSI_CFG_REG       0x0510
#define AHB3_CFG_REG      0x051c
#define APB1_CFG_REG      0x0520
#define APB2_CFG_REG      0x0524

#define CPUX_CLK_SRC(x)   ((x) << 24)
#define CPUX_APB_CLK_M(x) ((x) << 8)
#define CPUX_AXI_CLK_M(x) ((x) << 0)

#define PSI_CLK_SRC(x)    ((x) << 24)
#define PSI_CLK_P(x)      ((x) << 8)
#define PSI_CLK_M(x)      ((x) << 0)

#define AHB3_CLK_SRC(x)   ((x) << 24)
#define AHB3_CLK_P(x)     ((x) << 8)
#define AHB3_CLK_M(x)     ((x) << 0)

#define APB1_CLK_SRC(x)   ((x) << 24)
#define APB1_CLK_P(x)     ((x) << 8)
#define APB1_CLK_M(x)     ((x) << 0)

#define APB2_CLK_SRC(x)   ((x) << 24)
#define APB2_CLK_P(x)     ((x) << 8)
#define APB2_CLK_M(x)     ((x) << 0)

static DEFINE_FIXED_RATE(ccu_get_pll_periph0_rate, 600000000U)

static DEFINE_FIXED_PARENT(ccu_get_pll_ddr0, ccu, CLK_PLL_DDR0)

/*
 * While APB2 has a mux, assume its parent is OSC24M. Reparenting APB2
 * to PLL_PERIPH0 in Linux for faster UART clocks is unsupported.
 */
static DEFINE_FIXED_PARENT(ccu_get_apb2_parent, r_ccu, CLK_OSC24M)
static DEFINE_FIXED_PARENT(ccu_get_apb2, ccu, CLK_APB2)

static const struct ccu_clock ccu_clocks[SUN50I_H6_CCU_CLOCKS] = {
	[CLK_PLL_DDR0] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.reg        = 0x0010,
		.lock       = 28,
		.gate       = BITMAP_INDEX(0x0010, 31),
	},
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_pll_periph0_rate,
	},
	[CLK_APB2] = {
		.get_parent = ccu_get_apb2_parent,
		.get_rate   = ccu_get_parent_rate,
	},
	[CLK_MBUS] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x0540, 31),
		.reset      = BITMAP_INDEX(0x0540, 30),
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x071c, 0),
		.reset      = BITMAP_INDEX(0x071c, 16),
	},
	[CLK_DRAM] = {
		.get_parent = ccu_get_pll_ddr0,
		.get_rate   = ccu_get_parent_rate,
		.reg        = 0x0800,
		.update     = 27,
		.reset      = BITMAP_INDEX(0x0800, 30),
	},
	/* Reset requires re-training DRAM, so ignore it. */
	[CLK_BUS_DRAM] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x080c, 0),
	},
	[CLK_BUS_PIO] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
	},
#if CONFIG(SERIAL_DEV_UART0)
	[CLK_BUS_UART0] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x090c, 0),
		.reset      = BITMAP_INDEX(0x090c, 16),
	},
#elif CONFIG(SERIAL_DEV_UART1)
	[CLK_BUS_UART1] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x090c, 1),
		.reset      = BITMAP_INDEX(0x090c, 17),
	},
#elif CONFIG(SERIAL_DEV_UART2)
	[CLK_BUS_UART2] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x090c, 2),
		.reset      = BITMAP_INDEX(0x090c, 18),
	},
#elif CONFIG(SERIAL_DEV_UART3)
	[CLK_BUS_UART3] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x090c, 3),
		.reset      = BITMAP_INDEX(0x090c, 19),
	},
#endif
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_H6_CCU_CLOCKS),
	},
	.clocks = ccu_clocks,
	.regs   = DEV_CCU,
};

void
ccu_suspend(void)
{
	/* Set CPUX to LOSC (32kHz), APB to CPUX/4, AXI to CPUX/3. */
	mmio_write_32(DEV_CCU + CPUX_AXI_CFG_REG,
	              CPUX_CLK_SRC(1) |
	              CPUX_APB_CLK_M(3) |
	              CPUX_AXI_CLK_M(2));

	/* Set PSI/AHB1/AHB2 to LOSC/1 (32kHz). */
	mmio_write_32(DEV_CCU + PSI_CFG_REG,
	              PSI_CLK_SRC(1) |
	              PSI_CLK_P(0) |
	              PSI_CLK_M(0));

	/* Set AHB3 to LOSC/1 (32kHz). */
	mmio_write_32(DEV_CCU + AHB3_CFG_REG,
	              AHB3_CLK_SRC(1) |
	              AHB3_CLK_P(0) |
	              AHB3_CLK_M(0));

	/* Set APB1 to LOSC/2 (16kHz). */
	mmio_write_32(DEV_CCU + APB1_CFG_REG,
	              APB1_CLK_SRC(1) |
	              APB1_CLK_P(1) |
	              APB1_CLK_M(0));
}

void
ccu_resume(void)
{
	/* Set CPUX to PLL_CPUX, APB to CPUX/4, AXI to CPUX/3. */
	mmio_write_32(DEV_CCU + CPUX_AXI_CFG_REG,
	              CPUX_CLK_SRC(3) |
	              CPUX_APB_CLK_M(3) |
	              CPUX_AXI_CLK_M(2));

	/* Set PSI/AHB1/AHB2 to PLL_PERIPH0/3 (200MHz). */
	mmio_write_32(DEV_CCU + PSI_CFG_REG,
	              PSI_CLK_SRC(3) |
	              PSI_CLK_P(0) |
	              PSI_CLK_M(2));

	/* Set AHB3 to PLL_PERIPH0/3 (200MHz). */
	mmio_write_32(DEV_CCU + AHB3_CFG_REG,
	              AHB3_CLK_SRC(3) |
	              AHB3_CLK_P(0) |
	              AHB3_CLK_M(2));

	/* Set APB1 to PLL_PERIPH0/6 (100MHz). */
	mmio_write_32(DEV_CCU + APB1_CFG_REG,
	              APB1_CLK_SRC(3) |
	              APB1_CLK_P(1) |
	              APB1_CLK_M(2));
}

void
ccu_init(void)
{
	/* Set APB2 to OSC24M/1 (24MHz). */
	mmio_write_32(DEV_CCU + APB2_CFG_REG,
	              APB2_CLK_SRC(0) |
	              APB2_CLK_P(0) |
	              APB2_CLK_M(0));

	ccu_resume();
}
