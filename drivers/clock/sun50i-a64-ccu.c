/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <stdint.h>
#include <clock/sunxi-ccu.h>
#include <platform/devices.h>

#include "sunxi-ccu.h"

static struct sunxi_ccu_clock sun50i_a64_ccu_clocks[SUN50I_A64_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = FIXED_CLOCK("pll_periph0", 600000000, 0),
	[CLK_BUS_MSGBOX]  = {
		.info = {
			.name  = "msgbox",
			.flags = CLK_FIXED,
		},
		.gate  = BITMAP_INDEX(0x0064 >> 2, 21),
		.reset = BITMAP_INDEX(0x02c4 >> 2, 21),
	},
};

const struct sunxi_ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &sunxi_ccu_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clocks = sun50i_a64_ccu_clocks,
	.regs   = DEV_CCU,
};
