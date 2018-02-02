/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_CLOCK_SUNXI_CCU_H
#define DRIVERS_CLOCK_SUNXI_CCU_H

#include <stdint.h>

#define SUNXI_CCU_DRVDATA \
	(uintptr_t)&(struct sunxi_ccu_clock[])

struct sunxi_ccu_clock {
	/** Offset into the CCU of the clock gate bit, zero for none. */
	uint16_t gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	uint16_t reset;
};

extern const struct driver sunxi_ccu_driver;

#endif /* DRIVERS_CLOCK_SUNXI_CCU_H */
