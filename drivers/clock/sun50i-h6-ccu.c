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

static uint32_t
sun50i_h6_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                             const struct ccu_clock *clk UNUSED,
                             uint32_t rate UNUSED)
{
	return 600000000U;
}

static const struct ccu_clock sun50i_h6_ccu_clocks[SUN50I_H6_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun50i_h6_ccu_fixed_get_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x071c, 0),
		.reset      = BITMAP_INDEX(0x071c, 16),
	},
};

static const struct clock_driver sun50i_h6_ccu_driver = {
	.drv = {
		.probe   = dummy_probe,
		.release = dummy_release,
	},
	.ops = {
		.get_parent = ccu_get_parent,
		.get_rate   = ccu_get_rate,
		.get_state  = ccu_get_state,
		.set_state  = ccu_set_state,
	},
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &sun50i_h6_ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN50I_H6_CCU_CLOCKS),
	},
	.clocks = sun50i_h6_ccu_clocks,
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
	ccu_resume();
}
