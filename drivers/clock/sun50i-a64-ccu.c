/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <debug.h>
#include <device.h>
#include <error.h>
#include <stdint.h>
#include <clock/sunxi-ccu.h>
#include <platform/devices.h>

#include "sunxi-ccu.h"

static int
sun50i_a64_ccu_fixed_rate(const struct sunxi_ccu *self UNUSED,
                          uint8_t id UNUSED, uint32_t *rate)
{
	assert(id == CLK_PLL_PERIPH0);

	*rate = 600000000U;

	return SUCCESS;
}

static struct sunxi_ccu_clock sun50i_a64_ccu_clocks[SUN50I_A64_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.info.name = "pll_periph0",
		.get_rate  = sun50i_a64_ccu_fixed_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.info.name = "msgbox",
		.gate      = BITMAP_INDEX(0x0064 >> 2, 21),
		.reset     = BITMAP_INDEX(0x02c4 >> 2, 21),
	},
};

const struct sunxi_ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &sunxi_ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_A64_CCU_CLOCKS),
	},
	.clocks = sun50i_a64_ccu_clocks,
	.regs   = DEV_CCU,
};
