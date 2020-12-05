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
#include <platform/prcm.h>

#include "ccu.h"

#define CPUS_CLK_REG    0x0000

#define CPUS_CLK_SRC(x) ((x) << 24)
#define CPUS_PRE_DIV(x) ((x) << 8)
#define CPUS_CLK_P(x)   ((x) << 0)

/* Persist this var as r_ccu_init() may not be called after an exception. */
static uint32_t osc16m_rate = 16000000U;

static DEFINE_FIXED_RATE(r_ccu_get_osc16m_rate, osc16m_rate)
static DEFINE_FIXED_RATE(r_ccu_get_osc24m_rate, 24000000U)
static DEFINE_FIXED_RATE(r_ccu_get_osc32k_rate, 32768U)

static const struct clock_handle r_ccu_bus_parents[] = {
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC24M,
	},
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC32K,
	},
	{
		.dev = &r_ccu.dev,
		.id  = CLK_OSC16M,
	},
	{
		.dev = &ccu.dev,
		.id  = CLK_PLL_PERIPH0,
	},
};

static const struct clock_handle *
r_ccu_get_bus_parent(const struct ccu *self,
                     const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &r_ccu_bus_parents[bitfield_get(val, 24, 2)];
}

static uint32_t
r_ccu_get_mp_rate(const struct ccu *self,
                  const struct ccu_clock *clk, uint32_t rate)
{
	/* For AR100 and R_APB2, this assumes the pre-divider for PLL_PERIPH0
	 * (parent 3) will only be set if parent 3 is selected in the mux. */
	return ccu_helper_get_rate_mp(self, clk, rate, 0, 5, 8, 2);
}

static DEFINE_FIXED_PARENT(r_ccu_get_ar100, r_ccu, CLK_AR100)
static DEFINE_FIXED_PARENT(r_ccu_get_r_ahb, r_ccu, CLK_R_AHB)

static uint32_t
r_ccu_get_r_apb1_rate(const struct ccu *self,
                      const struct ccu_clock *clk, uint32_t rate)
{
	return ccu_helper_get_rate_m(self, clk, rate, 0, 2);
}

static DEFINE_FIXED_PARENT(r_ccu_get_r_apb1, r_ccu, CLK_R_APB1)
static DEFINE_FIXED_PARENT(r_ccu_get_r_apb2, r_ccu, CLK_R_APB2)

static const struct clock_handle r_ccu_module_parents[] = {
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
r_ccu_get_module_parent(const struct ccu *self,
                        const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &r_ccu_module_parents[bitfield_get(val, 24, 1)];
}

static const struct ccu_clock r_ccu_clocks[SUN50I_H6_R_CCU_CLOCKS] = {
	[CLK_OSC16M] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = r_ccu_get_osc16m_rate,
	},
	[CLK_OSC24M] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = r_ccu_get_osc24m_rate,
	},
	[CLK_OSC32K] = {
		.get_parent = ccu_get_null_parent,
		.get_rate   = r_ccu_get_osc32k_rate,
	},
	[CLK_AR100] = {
		.get_parent = r_ccu_get_bus_parent,
		.get_rate   = r_ccu_get_mp_rate,
		.reg        = CPUS_CLK_REG,
	},
	[CLK_R_AHB] = {
		.get_parent = r_ccu_get_ar100,
		.get_rate   = ccu_get_parent_rate,
	},
	[CLK_R_APB1] = {
		.get_parent = r_ccu_get_r_ahb,
		.get_rate   = r_ccu_get_r_apb1_rate,
		.reg        = 0x000c,
	},
	[CLK_R_APB2] = {
		.get_parent = r_ccu_get_bus_parent,
		.get_rate   = r_ccu_get_mp_rate,
		.reg        = 0x0010,
	},
	[CLK_BUS_R_PIO] = {
		.get_parent = r_ccu_get_r_apb1,
		.get_rate   = ccu_get_parent_rate,
	},
	[CLK_BUS_R_TIMER] = {
		.get_parent = r_ccu_get_r_apb1,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x011c, 0),
		.reset      = BITMAP_INDEX(0x011c, 16),
	},
	[CLK_BUS_R_TWD] = {
		/* Parent omitted to allow enabling before CCU init. */
		.get_parent = ccu_get_null_parent,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x012c, 0),
		.reset      = BITMAP_INDEX(0x012c, 16),
	},
	[CLK_BUS_R_PWM] = {
		.get_parent = r_ccu_get_r_apb1,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x013c, 0),
		.reset      = BITMAP_INDEX(0x013c, 16),
	},
	[CLK_BUS_R_UART] = {
		.get_parent = r_ccu_get_r_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x018c, 0),
		.reset      = BITMAP_INDEX(0x018c, 16),
	},
	[CLK_BUS_R_I2C] = {
		.get_parent = r_ccu_get_r_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x019c, 0),
		.reset      = BITMAP_INDEX(0x019c, 16),
	},
	[CLK_BUS_R_RSB] = {
		.get_parent = r_ccu_get_r_apb2,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x01bc, 0),
		.reset      = BITMAP_INDEX(0x01bc, 16),
	},
	[CLK_BUS_R_CIR] = {
		.get_parent = r_ccu_get_r_apb1,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x01cc, 0),
		.reset      = BITMAP_INDEX(0x01cc, 16),
	},
	[CLK_BUS_R_W1] = {
		.get_parent = r_ccu_get_r_apb1,
		.get_rate   = ccu_get_parent_rate,
		.gate       = BITMAP_INDEX(0x01ec, 0),
		.reset      = BITMAP_INDEX(0x01ec, 16),
	},
	[CLK_R_CIR] = {
		.get_parent = r_ccu_get_module_parent,
		.get_rate   = r_ccu_get_mp_rate,
		.reg        = 0x01c0,
		.gate       = BITMAP_INDEX(0x01c0, 31),
	},
	[CLK_R_W1] = {
		.get_parent = r_ccu_get_module_parent,
		.get_rate   = r_ccu_get_mp_rate,
		.reg        = 0x01e0,
		.gate       = BITMAP_INDEX(0x01e0, 31),
	},
};

const struct ccu r_ccu = {
	.dev = {
		.name  = "r_ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_H6_R_CCU_CLOCKS),
	},
	.clocks = r_ccu_clocks,
	.regs   = DEV_R_PRCM,
};

void
r_ccu_suspend(void)
{
	if (!CONFIG(SUSPEND_OSC24M))
		return;

	ccu_helper_disable_osc24m(PLL_CTRL_REG1);
}

void
r_ccu_resume(void)
{
	if (!CONFIG(SUSPEND_OSC24M))
		return;

	ccu_helper_enable_osc24m(PLL_CTRL_REG1);
}

void
r_ccu_init(void)
{
	/* Set CPUS to OSC16M/1 (16MHz). */
	mmio_write_32(DEV_R_PRCM + CPUS_CLK_REG,
	              CPUS_CLK_SRC(2) |
	              CPUS_PRE_DIV(0) |
	              CPUS_CLK_P(0));

	osc16m_rate = ccu_helper_calibrate_osc16m();
}
