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
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

#define CLK_AR100_REG 0x0000
#define CLK_APB0_REG  0x000c
#define CLK_R_CIR_REG 0x0054

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
sun8i_r_ccu_ar100_get_parent(const struct ccu *self, uint8_t id UNUSED)
{
	uint32_t val = mmio_read_32(self->regs + CLK_AR100_REG);

	return &sun8i_r_ccu_ar100_parents[bitfield_get(val, 16, 2)];
}

static uint32_t
sun8i_r_ccu_ar100_get_rate(const struct ccu *self,
                           uint32_t rate, uint8_t id UNUSED)
{
	uint32_t val = mmio_read_32(self->regs + CLK_AR100_REG);

	/* This assumes the pre-divider for PLL_PERIPH0 (parent 2)
	 * will only be set if parent 2 is selected in the mux. */
	return ccu_calc_rate_mp(val, rate, 8, 5, 4, 2);
}

static const struct clock_handle sun8i_r_ccu_ahb0_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_AR100,
};

static const struct clock_handle *
sun8i_r_ccu_ahb0_get_parent(const struct ccu *self UNUSED, uint8_t id UNUSED)
{
	return &sun8i_r_ccu_ahb0_parent;
}

static const struct clock_handle sun8i_r_ccu_apb0_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_AHB0,
};

static const struct clock_handle *
sun8i_r_ccu_apb0_get_parent(const struct ccu *self UNUSED, uint8_t id UNUSED)
{
	return &sun8i_r_ccu_apb0_parent;
}

static uint32_t
sun8i_r_ccu_apb0_get_rate(const struct ccu *self,
                          uint32_t rate, uint8_t id UNUSED)
{
	uint32_t val = mmio_read_32(self->regs + CLK_APB0_REG);

	return ccu_calc_rate_m(val, rate, 0, 2);
}

static const struct clock_handle sun8i_r_ccu_apb0_dev_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_APB0,
};

static const struct clock_handle *
sun8i_r_ccu_apb0_dev_get_parent(const struct ccu *self UNUSED,
                                uint8_t id UNUSED)
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
sun8i_r_ccu_r_cir_get_parent(const struct ccu *self, uint8_t id UNUSED)
{
	uint32_t val = mmio_read_32(self->regs + CLK_R_CIR_REG);

	return &sun8i_r_ccu_r_cir_parents[bitfield_get(val, 24, 1)];
}

static uint32_t
sun8i_r_ccu_r_cir_get_rate(const struct ccu *self,
                           uint32_t rate, uint8_t id UNUSED)
{
	uint32_t val = mmio_read_32(self->regs + CLK_R_CIR_REG);

	return ccu_calc_rate_mp(val, rate, 0, 4, 16, 2);
}

static const struct ccu_clock sun8i_r_ccu_clocks[SUN8I_R_CCU_CLOCKS] = {
	[CLK_OSC16M] = {
		.get_parent = ccu_get_parent_none,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC24M] = {
		.get_parent = ccu_get_parent_none,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_OSC32K] = {
		.get_parent = ccu_get_parent_none,
		.get_rate   = sun8i_r_ccu_fixed_get_rate,
	},
	[CLK_AR100] = {
		.get_parent = sun8i_r_ccu_ar100_get_parent,
		.get_rate   = sun8i_r_ccu_ar100_get_rate,
	},
	[CLK_AHB0] = {
		.get_parent = sun8i_r_ccu_ahb0_get_parent,
		.get_rate   = ccu_get_rate_parent,
	},
	[CLK_APB0] = {
		.get_parent = sun8i_r_ccu_apb0_get_parent,
		.get_rate   = sun8i_r_ccu_apb0_get_rate,
	},
	[CLK_BUS_R_PIO] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 0),
	},
	[CLK_BUS_R_CIR] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 1),
		.reset      = BITMAP_INDEX(0x00b0, 1),
	},
	[CLK_BUS_R_TIMER] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 2),
		.reset      = BITMAP_INDEX(0x00b0, 2),
	},
	[CLK_BUS_R_RSB] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 3),
		.reset      = BITMAP_INDEX(0x00b0, 3),
	},
	[CLK_BUS_R_UART] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 4),
		.reset      = BITMAP_INDEX(0x00b0, 4),
	},
	[CLK_BUS_R_I2C] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 6),
		.reset      = BITMAP_INDEX(0x00b0, 6),
	},
	[CLK_BUS_R_TWD] = {
		.get_parent = sun8i_r_ccu_apb0_dev_get_parent,
		.get_rate   = ccu_get_rate_parent,
		.gate       = BITMAP_INDEX(0x0028, 7),
	},
	[CLK_R_CIR] = {
		.get_parent = sun8i_r_ccu_r_cir_get_parent,
		.get_rate   = sun8i_r_ccu_r_cir_get_rate,
		.gate       = BITMAP_INDEX(CLK_R_CIR_REG, 31),
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
