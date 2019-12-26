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
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

static uint32_t
sun50i_a64_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                              uint32_t rate UNUSED, uint8_t id UNUSED)
{
	assert(id == CLK_PLL_PERIPH0);

	return 600000000U;
}

static const struct ccu_clock sun50i_a64_ccu_clocks[SUN50I_A64_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_rate = sun50i_a64_ccu_fixed_get_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.gate  = BITMAP_INDEX(0x0064 >> 2, 21),
		.reset = BITMAP_INDEX(0x02c4 >> 2, 21),
	},
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_A64_CCU_CLOCKS),
	},
	.clocks = sun50i_a64_ccu_clocks,
	.regs   = DEV_CCU,
};
