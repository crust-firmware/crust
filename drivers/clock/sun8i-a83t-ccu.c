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

#define APB2_CFG_REG    0x0058

#define APB2_CLK_SRC(x) ((x) << 24)
#define APB2_CLK_P(x)   ((x) << 16)
#define APB2_CLK_M(x)   ((x) << 0)

static DEFINE_FIXED_RATE(ccu_get_pll_periph0_rate, 600000000U)

/*
 * While APB2 has a mux, assume its parent is OSC24M. Reparenting APB2
 * to PLL_PERIPH0 in Linux for faster UART clocks is unsupported.
 */
static DEFINE_FIXED_PARENT(ccu_get_apb2_parent, r_ccu, CLK_OSC24M)
static DEFINE_FIXED_PARENT(ccu_get_apb2, ccu, CLK_APB2)

static const struct ccu_clock ccu_clocks[SUN8I_A83T_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_pll_periph0_rate,
	},
	[CLK_APB2] = {
		.get_parent = ccu_get_apb2_parent,
		.get_rate   = ccu_get_parent_rate,
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
#elif CONFIG(SERIAL_DEV_UART4)
	[CLK_BUS_UART4] = {
		.get_parent = ccu_get_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x006c, 20),
		.reset      = BITMAP_INDEX(0x02d8, 20),
	},
#endif
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_A83T_CCU_CLOCKS),
	},
	.clocks = ccu_clocks,
	.regs   = DEV_CCU,
};

void
ccu_suspend(void)
{
}

void
ccu_resume(void)
{
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
}
