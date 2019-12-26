/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CCU_PRIVATE_H
#define CCU_PRIVATE_H

#include <bitfield.h>
#include <clock.h>
#include <stdint.h>
#include <clock/ccu.h>

#include "clock.h"

struct ccu_clock {
	/** Handles to parent clocks (one for each possible mux value). */
	const struct clock_handle *parents;
	/** Hook for calculating the clock rate from the parent rate. */
	uint32_t                   (*get_rate)(const struct ccu *self,
	                                       uint32_t rate, uint8_t id);
	/** Offset into the CCU of the clock gate bit, zero for none. */
	uint16_t                   gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	uint16_t                   reset;
	/** Offset into the CCU of the mux/factor register. */
	uint16_t                   reg;
	/** Offset and width of the parent mux control in the register. */
	bitfield_t                 mux;
	/** Offset and width of the linear divider in the register. */
	bitfield_t                 m;
	/** Offset and width of the exponential divider in the register. */
	bitfield_t                 p;
};

extern const struct clock_driver ccu_driver;

#endif /* CCU_PRIVATE_H */
