/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_CLOCK_SUNXI_CCU_H
#define DRIVERS_CLOCK_SUNXI_CCU_H

#include <bitfield.h>
#include <stdint.h>
#include <util.h>

#define SUNXI_CCU_DRVDATA \
	(uintptr_t)&(struct sunxi_ccu_clock[])

#define SUNXI_CCU_NONE       UINT8_MAX
#define SUNXI_CCU_NO_PARENTS \
	{ SUNXI_CCU_NONE, SUNXI_CCU_NONE, SUNXI_CCU_NONE, SUNXI_CCU_NONE }
#define SUNXI_CCU_ONE_PARENT(parent) \
	{ parent, SUNXI_CCU_NONE, SUNXI_CCU_NONE, SUNXI_CCU_NONE }
#define SUNXI_CCU_PARENT_MAX 4

enum {
	/** This clock is the last entry in the driver data. */
	SUNXI_CCU_FLAG_LAST   = BIT(0),
	/** ARISC is not allowed to change this clock (it's owned by Linux). */
	SUNXI_CCU_FLAG_FIXED  = BIT(1),
	/** The clock has a gate in its register at bit 31. */
	SUNXI_CCU_FLAG_GATED  = BIT(2),
	/** The clock or its child has been explicitly enabled by a driver. */
	SUNXI_CCU_FLAG_ACTIVE = BIT(3),
};

struct sunxi_ccu_clock {
	/** Maximum allowed clock rate in Hz, zero for none. */
	uint32_t   max_rate;
	/** Current clock rate in Hz, zero for unknown. */
	uint32_t   rate;
	/** Offset into the CCU of the clock gate bit, zero for none. */
	uint16_t   gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	uint16_t   reset;
	/** Offset into the CCU of the mux/factor register. */
	uint16_t   reg;
	/** IDs of parent clocks, SUNXI_CCU_NONE for fixed/gate-only clocks. */
	uint8_t    parents[SUNXI_CCU_PARENT_MAX];
	/** Offset and width of the parent mux control in the register. */
	bitfield_t mux;
	/** Offset and width of the linear pre-divider for each parent. */
	bitfield_t pd[SUNXI_CCU_PARENT_MAX];
	/** Offset and width of the linear divider in the register. */
	bitfield_t m;
	/** Offset and width of the exponential divider in the register. */
	bitfield_t p;
	/** Some combination of SUNXI_CCU_FLAG flags (or none). */
	uint8_t    flags;
};

extern const struct driver sunxi_ccu_driver;

#endif /* DRIVERS_CLOCK_SUNXI_CCU_H */
