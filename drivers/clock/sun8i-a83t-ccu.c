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
sun8i_a83t_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                              const struct ccu_clock *clk UNUSED,
                              uint32_t rate UNUSED)
{
	return 600000000U;
}

static const struct ccu_clock sun8i_a83t_ccu_clocks[SUN8I_A83T_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun8i_a83t_ccu_fixed_get_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0064, 21),
		.reset      = BITMAP_INDEX(0x02c4, 21),
	},
};

static const struct clock_driver sun8i_a83t_ccu_driver = {
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
		.drv   = &sun8i_a83t_ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_A83T_CCU_CLOCKS),
	},
	.clocks = sun8i_a83t_ccu_clocks,
	.regs   = DEV_CCU,
};
