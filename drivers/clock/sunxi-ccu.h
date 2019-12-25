/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef SUNXI_CCU_PRIVATE_H
#define SUNXI_CCU_PRIVATE_H

#include <bitfield.h>
#include <clock.h>
#include <stdint.h>
#include <clock/sunxi-ccu.h>

#include "clock.h"

struct sunxi_ccu_clock {
	/** Handles to parent clocks (one for each possible mux value). */
	const struct clock_handle *parents;
	/** Hook for calculating the clock rate from the parent rate. */
	int                        (*get_rate)(const struct sunxi_ccu *self,
	                                       uint8_t id, uint32_t *rate);
	/** Offset into the CCU of the clock gate bit, zero for none. */
	const uint16_t             gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	const uint16_t             reset;
	/** Offset into the CCU of the mux/factor register. */
	const uint16_t             reg;
	/** Offset and width of the parent mux control in the register. */
	const bitfield_t           mux;
	/** Offset and width of the linear divider in the register. */
	const bitfield_t           m;
	/** Offset and width of the exponential divider in the register. */
	const bitfield_t           p;
};

extern const struct clock_driver sunxi_ccu_driver;

#endif /* SUNXI_CCU_PRIVATE_H */
