/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_CCU_H
#define DRIVERS_CLOCK_CCU_H

#include <clock.h>
#include <device.h>
#if CONFIG(PLATFORM_A64)
#include <clock/sun50i-a64-ccu.h>
#include <clock/sun8i-r-ccu.h>
#elif CONFIG(PLATFORM_A83T)
#include <clock/sun8i-a83t-ccu.h>
#include <clock/sun8i-r-ccu.h>
#elif CONFIG(PLATFORM_H6)
#include <clock/sun50i-h6-ccu.h>
#include <clock/sun50i-h6-r-ccu.h>
#endif

struct ccu {
	struct device           dev;
	const struct ccu_clock *clocks;
	uintptr_t               regs;
};

extern const struct ccu ccu;
extern const struct ccu r_ccu;

#endif /* DRIVERS_CLOCK_CCU_H */
