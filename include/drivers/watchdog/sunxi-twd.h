/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_SUNXI_TWD_H
#define DRIVERS_WATCHDOG_SUNXI_TWD_H

#include <clock.h>
#include <watchdog.h>

struct sunxi_twd {
	struct device       dev;
	struct clock_handle clock;
	uintptr_t           regs;
};

extern struct sunxi_twd r_twd;

#endif /* DRIVERS_WATCHDOG_SUNXI_TWD_H */
