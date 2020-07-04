/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
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

static uint32_t
sun50i_h6_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                             const struct ccu_clock *clk UNUSED,
                             uint32_t rate UNUSED)
{
	return 600000000U;
}

/*
 * APB2 has a mux, but it is assumed to always select OSC24M. Reparenting APB2
 * to PLL_PERIPH0 in Linux for faster UART clocks is unsupported.
 */
static const struct clock_handle sun50i_h6_ccu_apb2_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_OSC24M,
};

static const struct clock_handle *
sun50i_h6_ccu_apb2_get_parent(const struct ccu *self UNUSED,
                              const struct ccu_clock *clk UNUSED)
{
	return &sun50i_h6_ccu_apb2_parent;
}

static const struct ccu_clock sun50i_h6_ccu_clocks[SUN50I_H6_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_h6_ccu_fixed_get_rate,
	},
	[CLK_APB2] = {
		.get_parent = sun50i_h6_ccu_apb2_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x071c, 0),
		.reset      = BITMAP_INDEX(0x071c, 16),
	},
	[CLK_BUS_PIO] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_H6_CCU_CLOCKS),
	},
	.clocks = sun50i_h6_ccu_clocks,
	.regs   = DEV_CCU,
};

void
ccu_suspend(void)
{
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
