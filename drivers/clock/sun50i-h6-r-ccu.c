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

static const uint32_t sun50i_h6_r_ccu_fixed_rates[] = {
	[CLK_OSC16M] = 16000000U,
	[CLK_OSC24M] = 24000000U,
	[CLK_OSC32K] = 32768U,
};

static uint32_t
sun50i_h6_r_ccu_fixed_get_rate(const struct ccu *self,
                               const struct ccu_clock *clk,
                               uint32_t rate UNUSED)
{
	uintptr_t id = clk - self->clocks;

	assert(id < ARRAY_SIZE(sun50i_h6_r_ccu_fixed_rates));

	return sun50i_h6_r_ccu_fixed_rates[id];
}

static const struct clock_handle sun50i_h6_r_ccu_bus_parents[] = {
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
sun50i_h6_r_ccu_bus_get_parent(const struct ccu *self,
                               const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun50i_h6_r_ccu_bus_parents[bitfield_get(val, 24, 2)];
}

static uint32_t
sun50i_h6_r_ccu_mp_get_rate(const struct ccu *self,
                            const struct ccu_clock *clk, uint32_t rate)
{
	/* For AR100 and R_APB2, this assumes the pre-divider for PLL_PERIPH0
	 * (parent 3) will only be set if parent 3 is selected in the mux. */
	return ccu_helper_get_rate_mp(self, clk, rate, 0, 5, 8, 2);
}

static const struct clock_handle sun50i_h6_r_ccu_r_ahb_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_AR100,
};

static const struct clock_handle *
sun50i_h6_r_ccu_r_ahb_get_parent(const struct ccu *self UNUSED,
                                 const struct ccu_clock *clk UNUSED)
{
	return &sun50i_h6_r_ccu_r_ahb_parent;
}

static const struct clock_handle sun50i_h6_r_ccu_r_apb1_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_R_AHB,
};

static const struct clock_handle *
sun50i_h6_r_ccu_r_apb1_get_parent(const struct ccu *self UNUSED,
                                  const struct ccu_clock *clk UNUSED)
{
	return &sun50i_h6_r_ccu_r_apb1_parent;
}

static uint32_t
sun50i_h6_r_ccu_r_apb1_get_rate(const struct ccu *self,
                                const struct ccu_clock *clk, uint32_t rate)
{
	return ccu_helper_get_rate_m(self, clk, rate, 0, 2);
}

static const struct clock_handle sun50i_h6_r_ccu_r_apb1_dev_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_R_APB1,
};

static const struct clock_handle *
sun50i_h6_r_ccu_r_apb1_dev_get_parent(const struct ccu *self UNUSED,
                                      const struct ccu_clock *clk UNUSED)
{
	return &sun50i_h6_r_ccu_r_apb1_dev_parent;
}

static const struct clock_handle sun50i_h6_r_ccu_r_apb2_dev_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_R_APB2,
};

static const struct clock_handle *
sun50i_h6_r_ccu_r_apb2_dev_get_parent(const struct ccu *self UNUSED,
                                      const struct ccu_clock *clk UNUSED)
{
	return &sun50i_h6_r_ccu_r_apb2_dev_parent;
}

static const struct clock_handle sun50i_h6_r_ccu_module_parents[] = {
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
sun50i_h6_r_ccu_module_get_parent(const struct ccu *self,
                                  const struct ccu_clock *clk)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	return &sun50i_h6_r_ccu_module_parents[bitfield_get(val, 24, 1)];
}

static const struct ccu_clock sun50i_h6_r_ccu_clocks[SUN50I_H6_R_CCU_CLOCKS] =
{
	[CLK_OSC16M] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_h6_r_ccu_fixed_get_rate,
	},
	[CLK_OSC24M] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_h6_r_ccu_fixed_get_rate,
	},
	[CLK_OSC32K] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_h6_r_ccu_fixed_get_rate,
	},
	[CLK_AR100] = {
		.get_parent = sun50i_h6_r_ccu_bus_get_parent,
		.get_rate   = sun50i_h6_r_ccu_mp_get_rate,
		.reg        = 0x0000,
	},
	[CLK_R_AHB] = {
		.get_parent = sun50i_h6_r_ccu_r_ahb_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
	[CLK_R_APB1] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_get_parent,
		.get_rate   = sun50i_h6_r_ccu_r_apb1_get_rate,
		.reg        = 0x000c,
	},
	[CLK_R_APB2] = {
		.get_parent = sun50i_h6_r_ccu_bus_get_parent,
		.get_rate   = sun50i_h6_r_ccu_mp_get_rate,
		.reg        = 0x0010,
	},
	[CLK_BUS_R_PIO] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
	[CLK_BUS_R_TIMER] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x011c, 0),
		.reset      = BITMAP_INDEX(0x011c, 16),
	},
	[CLK_BUS_R_TWD] = {
		/* Parent omitted to allow enabling before CCU init. */
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x012c, 0),
		.reset      = BITMAP_INDEX(0x012c, 16),
	},
	[CLK_BUS_R_PWM] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x013c, 0),
		.reset      = BITMAP_INDEX(0x013c, 16),
	},
	[CLK_BUS_R_UART] = {
		.get_parent = sun50i_h6_r_ccu_r_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x018c, 0),
		.reset      = BITMAP_INDEX(0x018c, 16),
	},
	[CLK_BUS_R_I2C] = {
		.get_parent = sun50i_h6_r_ccu_r_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x019c, 0),
		.reset      = BITMAP_INDEX(0x019c, 16),
	},
	[CLK_BUS_R_CIR] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x01cc, 0),
		.reset      = BITMAP_INDEX(0x01cc, 16),
	},
	[CLK_BUS_R_W1] = {
		.get_parent = sun50i_h6_r_ccu_r_apb1_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x01ec, 0),
		.reset      = BITMAP_INDEX(0x01ec, 16),
	},
	[CLK_R_CIR] = {
		.get_parent = sun50i_h6_r_ccu_module_get_parent,
		.get_rate   = sun50i_h6_r_ccu_mp_get_rate,
		.reg        = 0x01c0,
		.gate       = BITMAP_INDEX(0x01c0, 31),
	},
	[CLK_R_W1] = {
		.get_parent = sun50i_h6_r_ccu_module_get_parent,
		.get_rate   = sun50i_h6_r_ccu_mp_get_rate,
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
	.clocks = sun50i_h6_r_ccu_clocks,
	.regs   = DEV_R_PRCM,
};

void
r_ccu_suspend(void)
{
}

void
r_ccu_resume(void)
{
}

void
r_ccu_init(void)
{
}
