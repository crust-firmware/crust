/*
 * Copyright © 2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_SUN6I_A31_WDT_H
#define DRIVERS_WATCHDOG_SUN6I_A31_WDT_H

#include <device.h>
#include <watchdog.h>

struct sun6i_a31_wdt {
	struct device dev;
	uintptr_t     regs;
};

extern const struct sun6i_a31_wdt r_wdog;

#endif /* DRIVERS_WATCHDOG_SUN6I_A31_WDT_H */
