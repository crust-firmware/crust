/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_SUNXI_CCU_H
#define DRIVERS_CLOCK_SUNXI_CCU_H

#include <clock.h>
#include <dm.h>
#include <platform/ccu.h>
#include <platform/r_ccu.h>

struct sunxi_ccu {
	struct device           dev;
	struct sunxi_ccu_clock *clocks;
};

extern struct sunxi_ccu ccu;
extern struct sunxi_ccu r_ccu;

#endif /* DRIVERS_CLOCK_SUNXI_CCU_H */
