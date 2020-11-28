/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <bitmap.h>
#include <clock.h>
#include <debug.h>
#include <device.h>
#include <mmio.h>
#include <stddef.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

#define CPUS_CLK_REG              0x0000
#define PLL_CTRL_REG1             0x0044
#define VDD_SYS_PWROFF_GATING_REG 0x0110

#define CPUS_CLK_SRC(x)           ((x) << 16)
#define CPUS_PRE_DIV(x)           ((x) << 8)
#define CPUS_CLK_P(x)             ((x) << 0)

static uint32_t sun8i_r_ccu_fixed_rates[] = {
	[CLK_OSC16M] = 16000000U,
	[CLK_OSC24M] = 24000000U,
	[CLK_OSC32K] = 32768U,
};

static uint32_t
sun8i_r_ccu_fixed_get_rate(const struct ccu *self,
                           const struct ccu_clock *clk, uint32_t rate UNUSED)
{
	uintptr_t id = clk - self->clocks;

	assert(id < ARRAY_SIZE(sun8i_r_ccu_fixed_rates));

	return sun8i_r_ccu_fixed_rates[id];
}

static const struct clock_handle sun8i_r_ccu_ar100_parents[] = {
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
		.id  = CLK_PLL_PERIPH0,
	},
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC16M,
	},
};

static const struct clock_handle *
sun8i_r_ccu_ar100_get_parent(const struct ccu *self,
                             const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun8i_r_ccu_ar100_parents[bitfield_get(val, 16, 2)];
}

static uint32_t
sun8i_r_ccu_ar100_get_rate(const struct ccu *self,
                           const struct ccu_clock *clk, uint32_t rate)
{
	/* This assumes the pre-divider for PLL_PERIPH0 (parent 2)
	 * will only be set if parent 2 is selected in the mux. */
	return ccu_helper_get_rate_mp(self, clk, rate, 8, 5, 4, 2);
}

static const struct clock_handle sun8i_r_ccu_ahb0_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_AR100,
};

static const struct clock_handle *
sun8i_r_ccu_ahb0_get_parent(const struct ccu *self UNUSED,
                            const struct ccu_clock *clk UNUSED)
{
	return &sun8i_r_ccu_ahb0_parent;
}

static const struct clock_handle sun8i_r_ccu_apb0_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_AHB0,
};

static const struct clock_handle *
sun8i_r_ccu_apb0_get_parent(const struct ccu *self UNUSED,
                            const struct ccu_clock *clk UNUSED)
{
	return &sun8i_r_ccu_apb0_parent;
}

static uint32_t
sun8i_r_ccu_apb0_get_rate(const struct ccu *self,
                          const struct ccu_clock *clk, uint32_t rate)
{
	return ccu_helper_get_rate_m(self, clk, rate, 0, 2);
}

static const struct clock_handle sun8i_r_ccu_apb0_dev_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_APB0,
};

static const struct clock_handle *
sun8i_r_ccu_apb0_dev_get_parent(const struct ccu *self UNUSED,
                                const struct ccu_clock *clk UNUSED)
{
	return &sun8i_r_ccu_apb0_dev_parent;
}

static const struct clock_handle sun8i_r_ccu_r_cir_parents[] = {
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC32K,
	},
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC24M,
	},
};

static const struct clock_handle *
sun8i_r_ccu_r_cir_get_parent(const struct ccu *self,
                             const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun8i_r_ccu_r_cir_parents[bitfield_get(val, 24, 1)];
}

static uint32_t
sun8i_r_ccu_r_cir_get_rate(const struct ccu *self,
                           const struct ccu_clock *clk, uint32_t rate)
{
	return ccu_helper_get_rate_mp(self, clk, rate, 0, 4, 16, 2);
}

static const struct ccu_clock sun8i_r_ccu_clocks[SUN8I_R_CCU_CLOCKS] = {
	[CLK_OSC16M] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC24M] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC32K] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_AR100] = {
		.get_parent = sun8i_r_ccu_ar100_get_parent,
		.get_rate   = sun8i_r_ccu_ar100_get_rate,
		.reg        = CPUS_CLK_REG,
	},
	[CLK_AHB0] = {
		.get_parent = sun8i_r_ccu_ahb0_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
	[CLK_APB0] = {
		.get_parent = sun8i_r_ccu_apb0_get_parent,
		.get_rate   = sun8i_r_ccu_apb0_get_rate,
		.reg        = 0x000c,
	},
	[CLK_BUS_R_PIO] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 0),
	},
	[CLK_BUS_R_CIR] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 1),
		.reset      = BITMAP_INDEX(0x00b0, 1),
	},
	[CLK_BUS_R_TIMER] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 2),
		.reset      = BITMAP_INDEX(0x00b0, 2),
	},
#if CONFIG(HAVE_RSB)
	[CLK_BUS_R_RSB] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 3),
		.reset      = BITMAP_INDEX(0x00b0, 3),
	},
#endif
	[CLK_BUS_R_UART] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 4),
		.reset      = BITMAP_INDEX(0x00b0, 4),
	},
	[CLK_BUS_R_I2C] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 6),
		.reset      = BITMAP_INDEX(0x00b0, 6),
	},
	[CLK_BUS_R_TWD] = {
		/* Parent omitted to allow enabling before CCU init. */
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0028, 7),
	},
	[CLK_R_CIR] = {
		.get_parent = sun8i_r_ccu_r_cir_get_parent,
		.get_rate   = sun8i_r_ccu_r_cir_get_rate,
		.reg        = 0x0054,
		.gate       = BITMAP_INDEX(0x0054, 31),
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

void
r_ccu_suspend(void)
{
	if (!CONFIG(SUSPEND_OSC24M))
		return;

	ccu_helper_disable_osc24m(DEV_R_PRCM + PLL_CTRL_REG1);
	if (CONFIG(PLATFORM_A64))
		mmio_set_32(DEV_R_PRCM + VDD_SYS_PWROFF_GATING_REG, BIT(2));
}

void
r_ccu_resume(void)
{
	if (!CONFIG(SUSPEND_OSC24M))
		return;

	if (CONFIG(PLATFORM_A64))
		mmio_clr_32(DEV_R_PRCM + VDD_SYS_PWROFF_GATING_REG, BIT(2));
	ccu_helper_enable_osc24m(DEV_R_PRCM + PLL_CTRL_REG1);
}

void
r_ccu_init(void)
{
	/* Set CPUS to OSC16M/1 (16MHz). */
	mmio_write_32(DEV_R_PRCM + CPUS_CLK_REG,
	              CPUS_CLK_SRC(3) |
	              CPUS_PRE_DIV(0) |
	              CPUS_CLK_P(0));

	sun8i_r_ccu_fixed_rates[CLK_OSC16M] = ccu_helper_calibrate_osc16m();
}
